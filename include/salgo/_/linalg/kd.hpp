#pragma once

#include "aabb.hpp"

#include "../key-val.hpp"
#include "../accessors.hpp"
#include "../const-flag.hpp"
#include "../template-macros.hpp"
#include "../add-member.hpp"
#include "../iteration-callback.hpp"

#include "../graph/n-ary-forest/n-ary-forest.hpp"
#include "../alloc/array-allocator.hpp"


#include <functional>

#include "../helper-macros-on.inc"

namespace salgo::linalg::_::kd {


template<class X>
using Aabb_Of = decltype( std::declval< Get_Aabb<X> >()( std::declval<X>() ) );




template<class PARAMS>
struct Node;


template<class KEY, class VAL, class AABB, class IMPLICIT_KEY, class BINARY_FOREST, int ALIGN_OVERRIDE, bool ERASABLE>
struct Params {
	using Key = KEY;
	using Val = VAL;
	using Aabb = AABB;
	using Implicit_Key = IMPLICIT_KEY;
	static constexpr auto Align_Override = ALIGN_OVERRIDE;
	static constexpr auto Erasable = ERASABLE;

	static constexpr auto Has_Key = ! std::is_same_v<Key, void>;
	static constexpr auto Has_Val = ! std::is_same_v<Val, void>;

	using Key_Val = salgo::Key_Val< Key, Val >;

	using Node = kd::Node<Params>;

	static constexpr auto Align = Align_Override != -1 ? Align_Override : BINARY_FOREST::Align;
	using Binary_Forest = typename BINARY_FOREST ::template DATA<Node> ::CHILD_LINKS ::template ALIGN<Align>;

	using Handle       = typename Binary_Forest::Handle;
	using Handle_Small = typename Binary_Forest::Handle_Small;

	using Vector = std::remove_cv_t< std::remove_reference_t< decltype( std::declval<Aabb>().min() ) > >;
	using Scalar = std::remove_reference_t< decltype( std::declval<Vector>()[0] ) >;
	static constexpr auto Dim = Vector::RowsAtCompileTime;
};




template<class PARAMS>
struct Context;

template<class PARAMS>
class Kd;



//
// accessor
//
template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C,Context<P>> {
	using BASE = Accessor_Base<C, Context<P> >;

	using BASE::BASE; // constructor
	friend Kd<P>;

public:
	// get key
	template<class PP = P, SALGO_REQUIRES(PP::Has_Key)>
	auto& key()       { static_assert(P::Has_Key, "your KD tree does not store keys"); return CONT._tree[ HANDLE ].kv.key; }

	template<class PP = P, SALGO_REQUIRES(PP::Has_Key)>
	auto& key() const { static_assert(P::Has_Key, "your KD tree does not store keys"); return CONT._tree[ HANDLE ].kv.key; }

	auto& key_val()       { return CONT._tree[ HANDLE ].kv; }
	auto& key_val() const { return CONT._tree[ HANDLE ].kv; }

	void erase() {
		static_assert(C==MUTAB, "erase() called on CONST accessor");
		static_assert(P::Erasable, "erase() only available if kd tree is ERASABLE");

		CONT._tree[HANDLE]._erased = true;
		// todo: destruct key/val, maybe even aabb
	}
};



//
// iterator
//
template<class PARAMS>
struct End_Iterator {};

template<class PARAMS, Const_Flag C>
class Iterator : public Iterator_Base<C,Context<PARAMS>> {
	using BASE = Accessor_Base<C, Context<PARAMS> >;

	using BASE::BASE; // constructor
	friend Kd<PARAMS>;

public:
	bool operator!=(End_Iterator<PARAMS>) const { DCHECK(false); return false; }
};




//
// accessor/iterator context
//
template<class P>
struct Context {
	using Container = Kd<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Accessor = kd::Accessor<P, C>;

	template<Const_Flag C>
	using Iterator = kd::Iterator<P, C>;
};

SALGO_ADD_MEMBER(_erased)

template<class P>
struct Node : Add__erased<bool, P::Erasable> {
	typename P::Aabb aabb; // for this key only
	typename P::Key_Val kv; // todo: keep inside Inplace_Storage to destruct after erase()

	typename P::Scalar l_to; // left to
	typename P::Scalar r_fr; // right from

	bool erased() const {
		if constexpr(P::Erasable) {
			return this->_erased;
		}
		else return false;
	}

	FORWARDING_CONSTRUCTOR_VAR(Node, kv) {}
};

SALGO_GENERATE_HAS_MEMBER(key)
SALGO_GENERATE_HAS_MEMBER(val)

SALGO_GENERATE_HAS_MEMBER(first)
SALGO_GENERATE_HAS_MEMBER(second)

//
// kd
//
template<class P>
class Kd : protected P {
public:
	using typename P::Scalar;
	using typename P::Vector;

	using typename P::Key;
	using typename P::Val;

	using typename P::Handle;
	using typename P::Handle_Small;

private:
	using typename P::Node;
	using typename P::Binary_Forest;
	using typename P::Aabb;

	using typename P::Key_Val;


	Binary_Forest _tree;
	Handle _root;

	friend Accessor<P, MUTAB>;
	friend Accessor<P, CONST>;

private:
	template<class AGG>
	static constexpr auto _is_kv_like_impl =   has_member__key<AGG> && has_member__val<AGG>;

	template<class AGG>
	static constexpr auto _is_pair_like_impl = has_member__first<AGG> && has_member__second<AGG>;

	template<class AGG>
	static constexpr auto _is_kv_like = _is_kv_like_impl< std::remove_reference_t<AGG> >;

	template<class AGG>
	static constexpr auto _is_pair_like = _is_pair_like_impl< std::remove_reference_t<AGG> >;

	template<class AGG>
	static constexpr auto _is_kv_or_pair_like =
		_is_kv_like<AGG> || _is_pair_like<AGG>;

public:

	template<class ARG, class... ARGS, SALGO_REQUIRES( ! _is_kv_or_pair_like<ARG> )>
	auto emplace(ARG&& arg, ARGS&&... args) {
		return _emplace( std::forward<ARG>(arg), std::forward<ARGS>(args)... );
	}

private:
	template<class KEY, class... ARGS>
	auto _select_tree_emplace(KEY&& key, ARGS&&... args) {
		if constexpr(P::Has_Key) {
			auto acc = _tree.emplace( key, std::forward<ARGS>(args)... );
			acc().aabb = Get_Aabb<KEY>()(key);
			return acc;
		}
		else {
			auto acc = _tree.emplace( std::forward<ARGS>(args)... );
			acc().aabb = Get_Aabb<KEY>()(key);
			return acc;
		}
	}

	template<class... ARGS>
	auto _emplace(ARGS&&... args) {
		// std::cout << "sizeof kd node " << sizeof(Node) << std::endl;
		auto new_node = _select_tree_emplace( std::forward<ARGS>(args)... );

		if(!_root.valid()) {
			_root = new_node;
			new_node().l_to = new_node().aabb.max()[0];
			new_node().r_fr = new_node().aabb.min()[0];
			return Accessor<P, MUTAB>(this, new_node);;
		}

		auto node = _tree( _root );
		int axis = 0;
		for(;;) {
			auto l_cost = new_node().aabb.max()[axis] - node().l_to;
			auto r_cost = node().r_fr - new_node().aabb.min()[axis];

			if(l_cost < r_cost) {
				node().l_to = std::max( node().l_to, new_node().aabb.max()[axis] );
				if(!node.has_left()) {
					node.link_left( new_node );
					break;
				}
				node = node.left();
			}
			else {
				node().r_fr = std::min( node().r_fr, new_node().aabb.min()[axis] );
				if(!node.has_right()) {
					node.link_right( new_node );
					break;
				}
				node = node.right();
			}
			static_assert(P::Dim > 0, "AABB dimensionality must be compile time constant");
			axis = (axis + 1) % P::Dim;
		}

		return Accessor<P, MUTAB>(this, new_node);
	}

public:
	auto insert(const typename P::Implicit_Key& key) {
		return emplace(key);
	}

	template<class V = Val>
	auto insert(const typename P::Implicit_Key& key, const V& val) {
		static_assert( P::Has_Val, "use insert(key) instead (or emplace(...))" );
		return emplace(key, val);
	}

public:
	// construct from {.key, .val}
	// construct from {.first, .second}
	template<class AGG, SALGO_REQUIRES( _is_kv_or_pair_like<AGG> )>
	auto emplace(AGG&& agg) {
		if constexpr( _is_kv_like<AGG> ) {
			return emplace( std::forward<AGG>(agg).key, std::forward<AGG>(agg).val ); // TODO: is this correct? does it correctly result with r-value ref?
		}
		if constexpr( _is_pair_like<AGG> ) {
			return emplace( std::forward<AGG>(agg).first, std::forward<AGG>(agg).second ); // TODO: is this correct? does it correctly result with r-value ref?
		}
	}


	auto find_closest_center(const Vector& vec) const {
		auto root = _tree( _root );
		if(!root.valid()) return Accessor<P, CONST>(this, Handle());

		auto best = std::get<0>( _find_closest_center_impl(vec, _tree(Handle()), std::numeric_limits<Scalar>::max(), root, 0) );

		return Accessor<P, CONST>(this, best);
	}

private:
	template<class VEC, class CAND_NODE>
	std::tuple<CAND_NODE, Scalar> _find_closest_center_impl(const VEC& vec, CAND_NODE best, Scalar best_dist_sqr, const CAND_NODE& node, int axis) const {

		if(!node().erased()) {
			auto cand_dist_sqr = (node().aabb.center() - vec).squaredNorm();
			if(cand_dist_sqr < best_dist_sqr) {
				best = node;
				best_dist_sqr = cand_dist_sqr;
			}
		}

		if(node.has_left()  && vec[axis] - node().l_to <= best_dist_sqr) { // TODO: add epsilon?
			std::tie(best, best_dist_sqr) = _find_closest_center_impl(vec, best, best_dist_sqr, node.left(), (axis+1) % P::Dim);
		}

		if(node.has_right() && node().r_fr - vec[axis] <= best_dist_sqr) { // TODO: add epsilon?
			std::tie(best, best_dist_sqr) = _find_closest_center_impl(vec, best, best_dist_sqr, node.right(), (axis+1) % P::Dim);
		}

		return std::tuple{best, best_dist_sqr};
	}

public:
	template<class AABB, class FUN>
	void each_intersecting(const AABB& aabb, const FUN& fun) {
		if(_root.not_valid()) return;

		if constexpr( std::is_same_v<void, std::invoke_result_t<FUN, Accessor<P,MUTAB>&>> ) {
			auto proxy_fun = [&fun](auto&&... args) {
				fun(std::forward<decltype(args)>(args)...);
				return CONTINUE;
			};
			_each_intersecting<MUTAB>(aabb, std::move(proxy_fun), _tree(_root), 0);
		}
		else {
			_each_intersecting<MUTAB>(aabb, fun, _tree(_root), 0);
		}
	}

	template<class AABB, class FUN>
	void each_intersecting(const AABB& aabb, const FUN& fun) const {
		if(_root.not_valid()) return;

		if constexpr( std::is_same_v<void, std::invoke_result_t<FUN, Accessor<P,CONST>&>> ) {
			auto proxy_fun = [&fun](auto&&... args) {
				fun(std::forward<decltype(args)>(args)...);
				return CONTINUE;
			};
			const_cast<Kd*>(this)->_each_intersecting<CONST>(aabb, std::move(proxy_fun), _tree(_root), 0);
		}
		else {
			const_cast<Kd*>(this)->_each_intersecting<CONST>(aabb, fun, _tree(_root), 0);
		}
	}

private:
	template<Const_Flag C, class AABB, class FUN, class ND>
	auto _each_intersecting(const AABB& aabb, const FUN& fun, const ND& node, int axis) {
		// warning: we're const-casted here, so need to respect `Const_Flag C`

		if(!node().erased()){
			if(node().aabb.intersects(aabb)) {
				auto accessor = Accessor<P, C>(this, node);
				auto r = fun( accessor );

				DCHECK(r == CONTINUE || r == BREAK) << "each_intersecting(a,fun): make sure you return value in every path of fun";
				if(r == BREAK) return BREAK;
			}
		}

		if(node.has_left() && node().l_to >= aabb.min()[axis]) {
			auto r = _each_intersecting<C>(aabb, fun, node.left(), (axis+1) % P::Dim);
			DCHECK(r == CONTINUE || r == BREAK) << "each_intersecting(a,fun): make sure you return value in every path of fun";
			if(r == BREAK) return BREAK;
		}

		if(node.has_right() && node().r_fr <= aabb.max()[axis]) {
			auto r = _each_intersecting<C>(aabb, fun, node.right(), (axis+1) % P::Dim);
			DCHECK(r == CONTINUE || r == BREAK) << "each_intersecting(a,fun): make sure you return value in every path of fun";
			if(r == BREAK) return BREAK;
		}

		return CONTINUE;
	}


public:
	auto operator()(const Handle& handle)       { return Accessor<P, MUTAB>(this, handle); }
	auto operator()(const Handle& handle) const { return Accessor<P, CONST>(this, handle); }

	template<class PP=P, SALGO_REQUIRES(PP::Has_Val)>
	auto& operator[](const Handle& handle)       { return _tree[ handle ].kv.val; }

	template<class PP=P, SALGO_REQUIRES(PP::Has_Val)>
	auto& operator[](const Handle& handle) const { return _tree[ handle ].kv.val; }
};




template<class P> class With_Builder;

//
// type builder
//
template<class P>
class Builder : private P {
	using typename P::Key;
	using typename P::Val;
	using typename P::Aabb;
	using typename P::Implicit_Key;
	using typename P::Binary_Forest;
	using P::Align_Override;
	using P::Erasable;

public:
	template<class X>
	using AABB          = With_Builder< Params<Key, Val, X, Implicit_Key, Binary_Forest, Align_Override, Erasable> >;

	template<class X>
	using AABB_OF       = With_Builder< Params<Key, Val, Aabb_Of<X>, X, Binary_Forest, Align_Override, Erasable> >;

	template<class X>
	using KEY           = With_Builder< Params<X, Val, Aabb_Of<X>, X, Binary_Forest, Align_Override, Erasable> >;

	template<class X>
	using VAL           = With_Builder< Params<Key, X, Aabb, Implicit_Key, Binary_Forest, Align_Override, Erasable> >;

	template<class X>
	using BINARY_FOREST = With_Builder< Params<Key, Val, Aabb, Implicit_Key, X, Align_Override, Erasable> >;

	template<int X>
	using ALIGN         = With_Builder< Params<Key, Val, Aabb, Implicit_Key, Binary_Forest, X, Erasable > >;

	using ERASABLE      = With_Builder< Params<Key, Val, Aabb, Implicit_Key, Binary_Forest, Align_Override, true > >;
};


template<class P>
class With_Builder : public Kd<P>, public Builder<P> {
	using BASE = Kd<P>;

public:
	using BASE::BASE;
};


template<class P>
class Bare_Builder : public Builder<P> {
	Bare_Builder() = delete;
	Bare_Builder(const Bare_Builder&) = delete;
	Bare_Builder(Bare_Builder&&) = delete;
};



} // namespace salgo::linalg::_::kd

#include "../helper-macros-off.inc"



namespace salgo::linalg {

using Kd = typename _::kd::Bare_Builder<_::kd::Params<
	void, // key
	void, // val
	Eigen::AlignedBox<float,13>, // aabb, will be rebound anyway
	void, // implicit key
	::salgo::graph::Binary_Forest,
	-1, // align-override
	false // erasable
>>;

} // namespace salgo::linalg


