#pragma once

#include "n-ary-forest/n-ary-forest.hpp"
#include "vector-allocator.hpp"
#include "get-aabb.hpp"
#include "key-val.hpp"
#include "accessors.hpp"
#include "const-flag.hpp"

#include "helper-macros-on"

namespace salgo {
namespace internal {
namespace kd {


template<class PARAMS>
struct Node;


template<class KEY, class VAL, class GET_AABB, class BINARY_FOREST>
struct Params {
	using Key = KEY;
	using Val = VAL;
	using Get_Aabb = GET_AABB;

	static constexpr auto Has_Val = ! std::is_same_v<Val, void>;

	using Key_Val = salgo::Key_Val<Key, Val>;

	using Node = kd::Node<Params>;

	using Binary_Forest = typename BINARY_FOREST ::template DATA<Node> ::CHILD_LINKS;

	using Handle       = typename Binary_Forest::Handle;
	using Handle_Small = typename Binary_Forest::Handle_Small;

	using Aabb = decltype( std::declval<GET_AABB>()(std::declval<KEY>()) );
	using Vector = std::remove_reference_t< decltype( std::declval<Aabb>().min() ) >;
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
	auto& key()       { return CONT._tree[ HANDLE ].kv.key; }
	auto& key() const { return CONT._tree[ HANDLE ].kv.key; }

	// get val (explicit)
	auto& val()       { static_assert(P::Has_Val, "your KD tree does not define any value (only keys)"); return BASE::operator()(); }
	auto& val() const { static_assert(P::Has_Val, "your KD tree does not define any value (only keys)"); return BASE::operator()(); }

	auto& key_val()       { return CONT._tree[ HANDLE ].kv; }
	auto& key_val() const { return CONT._tree[ HANDLE ].kv; }
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




template<class P>
struct Node {
	typename P::Aabb aabb; // for this key only
	typename P::Key_Val kv;

	typename P::Scalar l_to; // left to
	typename P::Scalar r_fr; // right from

	template<class... ARGS>
	Node(ARGS&&... args) : kv( std::forward<ARGS>(args)... ) {}
};

GENERATE_HAS_MEMBER(key)
GENERATE_HAS_MEMBER(val)

GENERATE_HAS_MEMBER(first)
GENERATE_HAS_MEMBER(second)

//
// kd
//
template<class P>
class Kd {
	using Node          = typename P::Node;
	using Binary_Forest = typename P::Binary_Forest;
	using Handle        = typename P::Handle;
	using Get_Aabb      = typename P::Get_Aabb;

	using Key = typename P::Key;
	using Val = typename P::Val;
	using Key_Val = typename P::Key_Val;

	using Scalar = typename P::Scalar;
	using Vector = typename P::Vector;

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
	auto emplace() {
		return _emplace();
	}

	template<class ARG, class... ARGS, REQUIRES( ! _is_kv_or_pair_like<ARG> )>
	auto emplace(ARG&& arg, ARGS&&... args) {
		return _emplace( std::forward<ARG>(arg), std::forward<ARGS>(args)... );
	}

private:
	template<class... ARGS>
	auto _emplace(ARGS&&... args) {
		// std::cout << "sizeof kd node " << sizeof(Node) << std::endl;
		auto new_node = _tree.emplace( std::forward<ARGS>(args)... );
		new_node().aabb = Get_Aabb()( new_node().kv.key );

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
	auto insert(const Key& key) {
		static_assert( ! P::Has_Val, "use insert(key,val) instead (or emplace(...))" );
		return emplace(key);
	}

	template<class V = Val>
	auto insert(const Key& key, const V& val) {
		static_assert( P::Has_Val, "use insert(key) instead (or emplace(...))" );
		return emplace(key, val);
	}

public:
	// construct from {.key, .val}
	// construct from {.first, .second}
	template<class AGG, REQUIRES( _is_kv_or_pair_like<AGG> )>
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
		if(!root.exists()) return Accessor<P, CONST>(this, root);

		auto best = std::get<0>( _find_closest_center_impl(vec, _tree(Handle()), std::numeric_limits<Scalar>::max(), root, 0) );

		return Accessor<P, CONST>(this, best);
	}

private:
	template<class VEC, class CAND_NODE>
	auto _find_closest_center_impl(const VEC& vec, CAND_NODE best, Scalar best_dist_sqr, const CAND_NODE& node, int axis) const {
		if(!node.exists()) return std::tuple{best, best_dist_sqr};

		auto cand_dist_sqr = (node().aabb.center() - vec).squaredNorm();
		if(cand_dist_sqr < best_dist_sqr) {
			best = node;
			best_dist_sqr = cand_dist_sqr;
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
		_each_intersecting<MUTAB>(aabb, fun, _tree(_root), 0);
	}

	template<class AABB, class FUN>
	void each_intersecting(const AABB& aabb, const FUN& fun) const {
		const_cast<Kd*>(this)->_each_intersecting<CONST>(aabb, fun, _tree(_root), 0);
	}

private:
	template<Const_Flag C, class AABB, class FUN, class ND>
	void _each_intersecting(const AABB& aabb, const FUN& fun, const ND& node, int axis) {
		// warning: we're const-casted here, so need to respect `Const_Flag C`
		{
			auto accessor = Accessor<P, C>(this, node);
			if(node().aabb.intersects(aabb)) fun( accessor );
		}

		if(node.has_left() && node().l_to >= aabb.min()[axis]) {
			_each_intersecting<C>(aabb, fun, node.left(), (axis+1) % P::Dim);
		}

		if(node.has_right() && node().r_fr <= aabb.max()[axis]) {
			_each_intersecting<C>(aabb, fun, node.right(), (axis+1) % P::Dim);
		}
	}

	// each_contained()
	// each_containing()

public:
	auto operator()(const Handle& handle)       { return Accessor<P, MUTAB>(this, handle); }
	auto operator()(const Handle& handle) const { return Accessor<P, CONST>(this, handle); }

	auto& operator[](const Handle& handle)       { return _tree[ handle ].kv.val; }
	auto& operator[](const Handle& handle) const { return _tree[ handle ].kv.val; }
};





//
// type builder
//
template<class P>
class With_Builder : public Kd<P> {
	using BASE = Kd<P>;

	using BASE::BASE;

	using Key           = typename P::Key;
	using Val           = typename P::Val;
	using Get_Aabb      = typename P::Get_Aabb;
	using Binary_Forest = typename P::Binary_Forest;

public:
	template<class X>
	using GET_AABB = With_Builder< Params<Key, Val, X, Binary_Forest> >;

	template<class X>
	using BINARY_FOREST = With_Builder< Params<Key, Val, Get_Aabb, X> >;

	template<int X>
	using ALIGN = With_Builder< Params<Key, Val, Get_Aabb, typename Binary_Forest ::template ALIGN<X> > >;
};



} // namespace kd
} // namespace internal




template<
	class KEY,
	class VAL = void
>
using Kd = typename internal::kd::With_Builder<internal::kd::Params<
	KEY,
	VAL,
	salgo::Get_Aabb<KEY>,
	salgo::Binary_Forest
>>;



} // namespace salgo

#include "helper-macros-off"

