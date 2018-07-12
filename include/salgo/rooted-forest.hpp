#pragma once

#include "stack-storage.hpp"
#include "const-flag.hpp"
#include "accessors.hpp"
#include "vector-allocator.hpp"
#include "key-val.hpp"




#include "helper-macros-on"
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
	using Handle_Small = typename Allocator :: Handle_Small;



	struct Node : Key_Val,
			Add_aggreg<Aggreg>,
			Add_propag<Propag>,
			Add_parent<Handle_Small, Parents> {

		FORWARDING_CONSTRUCTOR(Node, Key_Val) {}

		using Key_Val::operator();

		std::conditional_t<
			N_Ary==0,
			std::vector<Handle_Small>,
			std::array<Handle_Small, N_Ary>
		> children;
	};




	template<Const_Flag C>
	class Reference : public Reference_Base<C,Context> {
		using BASE = Reference_Base<C,Context>;

	public:
		FORWARDING_CONSTRUCTOR(Reference, BASE) {}

	protected:
		bool just_erased = false;
		void on_erase() {
			cached_parent = std::move( NODE.parent );
			cached_children = std::move( NODE.children );
			just_erased = true;
		}

		auto get_parent() const {
			if(just_erased) return cached_parent;
			else return NODE.parent;
		}

		const auto& get_children() const {
			if(just_erased) return cached_children;
			else return NODE.children;
		}

	private:
		Handle_Small cached_parent;
		decltype(Node::children) cached_children;
	};




	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;

		friend Rooted_Forest;

		friend Accessor<CONST>;
		friend Accessor<MUTAB>;

	public:
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

	public:
		auto& key()       { return NODE.key; }
		auto& key() const { return NODE.key; }

		auto& val()       { return NODE.val; }
		auto& val() const { return NODE.val; }

	public:
		bool exists() const { return HANDLE.valid(); }


		auto child(int ith)       { _check_child_index(ith);
				return Accessor<C    >( &CONT, BASE::get_children()[ith] ); }

		auto child(int ith) const { _check_child_index(ith);
				return Accessor<CONST>( &CONT, BASE::get_children()[ith] ); }

		auto left()        { static_assert(N_Ary == 2); return child(0); }
		auto left()  const { static_assert(N_Ary == 2); return child(0); }

		auto right()       { static_assert(N_Ary == 2); return child(1); }
		auto right() const { static_assert(N_Ary == 2); return child(1); }

		auto parent()       { return Accessor<C    >( &CONT, BASE::get_parent() ); }
		auto parent() const { return Accessor<CONST>( &CONT, BASE::get_parent() ); }



		bool is_ith_child(int ith) const {
			auto par = BASE::get_parent(); DCHECK(par.valid());
			parent()._check_child_index(ith);
			return HANDLE == _node(par).children[ith];
		}

		auto& is_which() const {
			auto par = BASE::get_parent(); DCHECK(par.valid());
			for(auto& ch : _node(par).children) {
				if(ch == BASE::_handle()) return ch;
			}
			DCHECK(false);
			return _node(par).children[0]; // just to supress warning
		}

		bool is_left() const {
			static_assert(N_Ary == 2);
			return is_ith_child(0);
		}

		bool is_right() const {
			static_assert(N_Ary == 2);
			return is_ith_child(0);
		}



		template<class... Args>
		auto emplace_child(int ith, Args&&... args) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( !child(ith).exists() );
			auto new_node = ALLOC.construct( std::forward<Args>(args)... );
			link_child(ith, new_node);
			return child(ith);
		}

		template<class... Args>
		auto emplace_left (Args&&... args) { return emplace_child(0, std::forward<Args>(args)...); }

		template<class... Args>
		auto emplace_right(Args&&... args) { return emplace_child(1, std::forward<Args>(args)...); }


		void link_child(int ith, Handle new_child) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );
			_check_child_index(ith);

			DCHECK( !child(ith).exists() ); // no child
			DCHECK( !BASE::_container()(new_child).parent().exists() ); // new_child has no parent

			NODE.children[ith] = new_child;
			_node(new_child).parent = HANDLE;
		}

		void link_left (Handle new_child) { static_assert(N_Ary == 2); link_child(0, new_child); }
		void link_right(Handle new_child) { static_assert(N_Ary == 2); link_child(1, new_child); }


		// automatically unlink things if needed (slower)
		void relink_child(int ith, Handle new_child) {
			if(_node(new_child).parent.valid()) BASE::_container()(new_child).unlink_parent();
			if(NODE.children[ith].valid()) unlink_child(ith);

			link_child(ith, new_child);
		}



	private:
		void _unlink_child_1way(Handle_Small& ch) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );

			DCHECK( ch.valid() );
			_node(ch).parent.reset();
		}

	public:
		void unlink_child(int ith) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );

			auto& ch = _node().children[ith];
			_unlink_child_1way(ch);
			ch.reset();
		}

		void unlink_left()  { unlink_child(0); }
		void unlink_right() { unlink_child(1); }



		// NOTE: linear in the number of children!
	private:
		void _unlink_parent_1way() {
			DCHECK( exists() );
			auto& par = _node( NODE.parent );
			DCHECK( std::find(par.children.begin(), par.children.end(), HANDLE) != par.children.end() );
			for(auto& ch : par.children) if(ch == HANDLE) {
				ch.reset();
				DCHECK_GT(N_Ary, 0) << "not implemented";
				// TODO: don't leave holes when N_ARY==0 (dynamic)
				// maybe use Unordered_Vector or Vector_Allocator instead for children list
				break;
			}
		}
	public:
		void unlink_parent() {
			_unlink_parent_1way();
			_node().parent.reset();
		}


	private:
		// prerequisite: no element points to this element (not checked)
		// when unlink_and_erase was just called, links will be 1-way
		void _erase_unchecked() {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );
			BASE::on_erase(); // cache links before removing this node
			ALLOC( HANDLE ).destruct();
		}

	public:
		void erase() {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );
			DCHECK( !NODE.parent.valid() ) << "can't erase, still has parent";

			for(auto& ch : NODE.children) DCHECK( !ch.valid() ) << "can't erase, still has child";

			_erase_unchecked();
		}

		void unlink_and_erase() {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( exists() );

			// unlink parent (if present)
			if( NODE.parent.valid() ) _unlink_parent_1way();

			// unlink children
			for(auto& ch : NODE.children) {
				if(ch.valid()) _unlink_child_1way(ch);
			}

			_erase_unchecked();
		}


	private:
		void _check_child_index(int ith) const {
			DCHECK_GE(ith, 0); DCHECK_LT(ith, BASE::get_children().size());
		}

		//void aggregate() {
		//	static_assert(C == MUTAB, "called aggregate() on CONST accessor");
		//	_node().aggreg = Aggreg();
		//	if() _node().aggreg.aggregate();
		//}

	private:
		auto& _node(Handle h)       { return ALLOC[ h ]; }
		auto& _node(Handle h) const { return ALLOC[ h ]; }
	};




	class End_Iterator {};

	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;

	public:
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() {
			HANDLE = ++ALLOC( HANDLE ).iterator();
		}

		void _decrement() {
			HANDLE = --ALLOC( HANDLE ).iterator();
		}

	public:
		bool operator!=(End_Iterator) const { return ALLOC( HANDLE ).iterator() != ALLOC.end(); }
	};












	class Rooted_Forest : private Allocator {
	public:
		using Key = Context::Key;
		using Val = Context::Val;
		using Key_Val = Context::Key_Val;

		using       Handle = Context::      Handle;
		using Handle_Small = Context::Handle_Small;

		template<Const_Flag C> using Accessor = Context::Accessor<C>;
		template<Const_Flag C> using Iterator = Context::Iterator<C>;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;

		friend Iterator<MUTAB>;
		friend Iterator<CONST>;

		friend Reference<MUTAB>;
		friend Reference<CONST>;


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
		auto begin()       { return Iterator<MUTAB>(this, _alloc().begin()); }
		auto begin() const { return Iterator<CONST>(this, _alloc().begin()); }

		auto end() const { return End_Iterator(); }
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
#include "helper-macros-off"

