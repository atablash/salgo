#pragma once

#include "stack-storage.hpp"
#include "const-flag.hpp"
#include "accessors.hpp"
#include "vector-allocator.hpp"
#include "key-val.hpp"


namespace salgo {






namespace internal {
namespace rooted_forest {

struct Single_Arg_Tag {};

//
// Aggreg::Aggreg() { create neutral Aggreg; }
// Aggreg::aggregate(const Aggreg& other) { add other to this; }
//
//
// Propag::Propag() { create neutral Propag; }
// Propag::propagate(Propag& other) const { add this to other, assuming other's range is prefix of this; }
// Propag::apply(Aggreg& aggreg) {
//     add this to aggreg, assuming aggreg's range is prefix of this;
//     update this so it's a valid Propag for the range without 'aggreg' prefix;
// }
//



//
// add AGGREG
//
template<class T> struct Add_aggreg { T aggreg; };
template<> struct Add_aggreg<void> {};



//
// add PROPAG
//
template<class T> struct Add_propag { T propag; };
template<> struct Add_propag<void> {};



template<class T, bool> struct Add_parent          { T parent; };
template<class T>       struct Add_parent<T,false> {};




template<
	int N_ARY,
	class _KEY,	class _VAL,
	bool _PARENTS,
	bool _EVERSIBLE,
	class _AGGREG, class _PROPAG,
	bool _PATH_PARENTS, class _PP_AGGREG, class _PP_PROPAG,
	class _ALLOCATOR
>
struct Context {
	static constexpr int N_Ary = N_ARY;

	using Key = _KEY;
	static constexpr bool Has_Key = !std::is_same_v<Key, void>;

	using Val = _VAL;
	static constexpr bool Has_Val = !std::is_same_v<Val, void>;

	using Key_Val = salgo::Key_Val<Key,Val>;

	static constexpr bool Parents = _PARENTS;

	static constexpr bool Eversible = _EVERSIBLE;

	using Aggreg = _AGGREG;
	using Propag = _PROPAG;
	static constexpr bool Has_Aggreg = !std::is_same_v<Aggreg,void>;
	static constexpr bool Has_Propag = !std::is_same_v<Propag,void>;

	static constexpr bool Path_Parents = _PATH_PARENTS;

	using Pp_Aggreg = _PP_AGGREG;
	using Pp_Propag = _PP_PROPAG;
	static constexpr bool Has_Pp_Aggreg = !std::is_same_v<Pp_Aggreg,void>;
	static constexpr bool Has_Pp_Propag = !std::is_same_v<Pp_Propag,void>;


	struct Node;
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Rooted_Forest;
	using Container = Rooted_Forest;

	using Allocator = typename _ALLOCATOR ::template VAL<Node>;

	using       Handle = typename Allocator ::       Handle;
	using Small_Handle = typename Allocator :: Small_Handle;



	struct Node : Key_Val,
			Add_aggreg<Aggreg>,
			Add_propag<Propag>,
			Add_parent<Small_Handle, Parents> {

		FORWARDING_CONSTRUCTOR(Node, Key_Val) {}

		std::conditional_t<
			N_Ary==0,
			std::vector<Small_Handle>,
			std::array<Small_Handle, N_Ary>
		> children;
	};








	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

		friend Rooted_Forest;

	public:
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

	public:
		auto& key()       { return _container()[_handle()].key; }
		auto& key() const { return _container()[_handle()].key; }

		auto& val()       { return _container()[_handle()].val; }
		auto& val() const { return _container()[_handle()].val; }

	public:
		bool exists() const { return _handle().valid(); }


		auto child(int ith)       { _check_child_index(ith); return Accessor<C    >( &_container(), _node().children[ith] ); }
		auto child(int ith) const { _check_child_index(ith); return Accessor<CONST>( &_container(), _node().children[ith] ); }

		auto left()        { return child(0); }
		auto left()  const { return child(0); }

		auto right()       { return child(1); }
		auto right() const { return child(1); }

		auto parent()       { return Accessor<C    >( &_container(), _node().parent ); }
		auto parent() const { return Accessor<CONST>( &_container(), _node().parent ); }



		bool has_child(int ith) const { _check_child_index(ith); return _node().children[ith].valid(); }

		bool has_left()   const { return has_child(0); }
		bool has_right()  const { return has_child(1); }
		bool has_parent() const { return _node().parent.valid(); }



		bool is_ith_child(int ith) const {
			auto par = _node().parent; DCHECK(par.valid());
			DCHECK(parent()._check_child_index(ith));
			return _handle() == _node(par).children[ith];
		}

		bool is_left() const {
			auto par = _node().parent;
			DCHECK(par.valid());
			return _handle() == _node(par).children[0];
		}

		bool is_right() const {
			auto par = _node().parent;
			DCHECK(par.valid());
			return _handle() == _node(par).children[1];
		}



		template<class... Args>
		auto emplace_child(int ith, Args&&... args) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( !child(ith).exists() );
			auto new_node = _container()._alloc().construct( std::forward<Args>(args)... );
			link_child(ith, new_node);
			return child(ith);
		}

		template<class... Args>
		auto emplace_left (Args&&... args) { return emplace_child(0, std::forward<Args>(args)...); }

		template<class... Args>
		auto emplace_right(Args&&... args) { return emplace_child(1, std::forward<Args>(args)...); }


		void link_child(int ith, Handle new_child) {
			static_assert(C == MUTAB, "called on CONST accessor");
			_check_child_index(ith);
			DCHECK( !child(ith).exists() );
			_node().children[ith] = new_child;
			_node(new_child).parent = _handle();
		}

		void link_left (Handle new_child) { link_child(0, new_child); }
		void link_right(Handle new_child) { link_child(1, new_child); }


		void unlink_child(int ith) {
			static_assert(C == MUTAB, "called on CONST accessor");
			auto& ch = _node().children[ith];
			DCHECK( ch.valid() );
			_node().children[ch].parent.reset();
			ch.reset();
		}

		void unlink_left()  { unlink_child(0); }
		void unlink_right() { unlink_child(1); }

		// NOTE: linear in the number of children!
		void unlink_parent() {
			auto& par = _node( _node().parent );
			DCHECK( std::find(par.children.begin(), par.children.end(), _handle()) != par.children.end() );
			for(auto& ch : par.children) if(ch == _handle()) {
				ch.reset();
				// TODO: don't leave holes when N_ARY==0 (dynamic)
				// maybe use Unordered_Vector or Vector_Allocator instead for children list
				break;
			}
			_node().parent.reset();
		}


		void erase() {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );
			DCHECK(!_node().parent.valid());

			for(auto& ch : _node().children) DCHECK( !ch.valid() );
			_container()._alloc().destruct( _handle() );
		}

		void unlink_and_erase() {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );

			// unlink parent (if present)
			if( _node().parent.valid() ) unlink_parent();

			// unlink children
			for(auto& ch : _node().children) {
				if(ch.valid()) unlink_child(ch);
			}

			erase();
		}


	private:
		void _check_child_index(int ith) const {
			DCHECK_GE(ith, 0); DCHECK_LT(ith, _node().children.size());
		}

		//void aggregate() {
		//	static_assert(C == MUTAB, "called aggregate() on CONST accessor");
		//	_node().aggreg = Aggreg();
		//	if() _node().aggreg.aggregate();
		//}

	private:
		auto& _node()       { return _container()._alloc()[_handle()]; }
		auto& _node() const { return _container()._alloc()[_handle()]; }
		auto& _node(Handle h)       { return _container()._alloc()[h]; }
		auto& _node(Handle h) const { return _container()._alloc()[h]; }
	};





	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;

	protected:
		using BASE::_container;
		using BASE::_handle;

	public:
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() {
			_handle() = ++_container()._alloc()( _handle() ).iterator();
		}

		void _decrement() {
			_handle() = --_container()._alloc()( _handle() ).iterator();
		}
	};












	class Rooted_Forest : private Allocator {
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;
		friend Iterator<MUTAB>;
		friend Iterator<CONST>;

	public:
		using Key = Context::Key;
		using Val = Context::Val;
		using Key_Val = Context::Key_Val;

		using       Handle = Context::      Handle;
		using Small_Handle = Context::Small_Handle;

		template<Const_Flag C> using Accessor = Context::Accessor<C>;
		template<Const_Flag C> using Iterator = Context::Iterator<C>;


	private:
		auto& _alloc()       { return *static_cast<      Allocator*>(this); }
		auto& _alloc() const { return *static_cast<const Allocator*>(this); }

	public:
		static_assert(Allocator::Auto_Destruct, "Rooted_Forest requires an Auto_Destruct allocator");

		bool empty() const {
			return _alloc().empty();
		}

		template<class... Args>
		auto emplace(Args&&... args) {
			auto new_node = _alloc().construct( std::forward<Args>(args)... );
			return Accessor<MUTAB>(this, new_node);
		}

		/*
		auto bst_emplace(const Key_Val& kv) {
			if constexpr(Has_Val) return bst_emplace( kv.key, kv.val );
			else return bst_emplace( kv.key );
		}

		template<class K, class... V>
		auto bst_emplace(K&& k, V&&... v) {
			if(empty()) return emplace_root( std::forward<K>(k), std::forward<V>(v)... );

			auto handle = _alloc()[super_root].left;
			DCHECK( handle.valid() );
			for(;;) {
				if(_alloc()[handle].get().key < k) {
					if(!_alloc()[handle].right.valid()) {
						auto new_node = _alloc().construct();
						new_node().construct( std::forward<K>(k), std::forward<V>(v)... );
						_alloc()[handle].right = new_node.handle();
						_alloc()[new_node].parent = handle;
						return Accessor<MUTAB>(this, new_node);
					}
					handle = _alloc()[handle].right;
				}
				else {
					if(!_alloc()[handle].left.valid()) {
						auto new_node = _alloc().construct();
						new_node().construct( std::forward<K>(k), std::forward<V>(v)... );
						_alloc()[handle].left = new_node.handle();
						_alloc()[new_node].parent = handle;
						return Accessor<MUTAB>(this, new_node);
					}
					handle = _alloc()[handle].left;
				}
			}
		}
		*/

	public:
		auto& operator[](Handle handle)       { return _alloc()[handle](); }
		auto& operator[](Handle handle) const { return _alloc()[handle](); }

		auto operator()(Handle handle)       { return Accessor<MUTAB>(this, handle); }
		auto operator()(Handle handle) const { return Accessor<CONST>(this, handle); }

	public:
		auto before_begin()       { return Iterator<MUTAB>(this, _alloc().before_begin()); }
		auto before_begin() const { return Iterator<CONST>(this, _alloc().before_begin()); }

		auto begin()       { return Iterator<MUTAB>(this, _alloc().begin()); }
		auto begin() const { return Iterator<CONST>(this, _alloc().begin()); }

		auto end()       { return Iterator<MUTAB>(this, _alloc().end()); }
		auto end() const { return Iterator<CONST>(this, _alloc().end()); }
	};



	struct With_Builder : Rooted_Forest {
		FORWARDING_CONSTRUCTOR(With_Builder, Rooted_Forest) {}

		template<class NEW_ALLOCATOR>
		using ALLOCATOR = typename Context<N_Ary, Key, Val, Parents, Eversible, Aggreg, Propag,
			Path_Parents, Pp_Aggreg, Pp_Propag, NEW_ALLOCATOR> ::With_Builder;
	};


}; // struct Context


} // rooted_forest
} // internal




// N_ARY is number of children per node
// if N_ARY==0, number of children is dynamic
// when called with 1 template argument,  assume it's Rooted_Forest<N_ARY> (no key or value)
// when called with 2 template arguments, assume it's Rooted_Forest<N_ARY, VAL> (no key)
// when called with 3 template arguments, assume it's Rooted_Forest<N_ARY, KEY, VAL>
template<int N_ARY, class KEY_OR_VAL = void, class VAL_OR_TAG = internal::rooted_forest::Single_Arg_Tag>
using Rooted_Forest = typename internal::rooted_forest::Context<
	N_ARY,
	std::conditional_t<!std::is_same_v<VAL_OR_TAG, internal::rooted_forest::Single_Arg_Tag>, KEY_OR_VAL, void>,
	std::conditional_t<!std::is_same_v<VAL_OR_TAG, internal::rooted_forest::Single_Arg_Tag>, VAL_OR_TAG, KEY_OR_VAL>,
	true, // parents
	false, // eversible
	void, // aggreg
	void, // propag
	false, // path parents
	void, // pp_aggreg
	void, // pp_propag
	Vector_Allocator<>
> :: With_Builder;



template<class KEY_OR_VAL = void, class VAL_OR_TAG = internal::rooted_forest::Single_Arg_Tag>
using Binary_Forest = Rooted_Forest<2, KEY_OR_VAL, VAL_OR_TAG>;






} // salgo