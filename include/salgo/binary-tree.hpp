#pragma once

#include "stack-storage.hpp"
#include "const-flag.hpp"
#include "accessors.hpp"
#include "random-allocator.hpp"
#include "key-val.hpp"


namespace salgo {


namespace internal {
namespace binary_tree {


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
template<class T> struct Add_aggreg {
	typename salgo::Stack_Storage<T> ::PERSISTENT aggreg;

	template<class... Args>
	void construct(Args&&... args) { aggreg.construct( std::forward<Args>(args)... ); }

	void destruct() { aggreg.destruct(); }
};
template<> struct Add_aggreg<void> {
	void construct(...) {}
	void destruct() {}
};



//
// add PROPAG
//
template<class T> struct Add_propag {
	typename salgo::Stack_Storage<T> ::PERSISTENT propag;

	template<class... Args>
	void construct(Args&&... args) { propag.construct( std::forward<Args>(args)... ); }

	void destruct() { propag.destruct(); }
};
template<> struct Add_propag<void> {
	void construct(...) {}
	void destruct() {}
};







template<
	class _KEY,	class _VAL,
	class _AGGREG, class _PROPAG,
	bool _PATH_PARENTS, class _PP_AGGREG, class _PP_PROPAG,
	class _ALLOCATOR
>
struct Context {
	using Key = _KEY;
	static constexpr bool Has_Key = !std::is_same_v<Key, void>;

	using Val = _VAL;
	static constexpr bool Has_Val = !std::is_same_v<Val, void>;

	using Key_Val = salgo::Key_Val<Key,Val>;

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
	class Binary_Tree;
	using Container = Binary_Tree;

	using Allocator = typename _ALLOCATOR ::template VAL<Node>;

	using       Handle = typename Allocator ::       Handle;
	using Small_Handle = typename Allocator :: Small_Handle;







	struct Node : salgo::Stack_Storage<Key_Val>::PERSISTENT,
			Add_aggreg<Aggreg>,
			Add_propag<Propag> {

	private:
		using KEY_VAL_BASE = typename salgo::Stack_Storage<Key_Val>::PERSISTENT;
		using AGGREG_BASE = Add_aggreg<Aggreg>;
		using PROPAG_BASE = Add_propag<Propag>;

	public:
		template<class... Args>
		void construct(Args&&... args) {
			KEY_VAL_BASE::construct( std::forward<Args>(args)... );
			AGGREG_BASE::construct();
			PROPAG_BASE::construct();
		}

		void destruct() {
			KEY_VAL_BASE::destruct();
			AGGREG_BASE::destruct();
			PROPAG_BASE::destruct();
		}

		Small_Handle parent;

		Small_Handle left;
		Small_Handle right;
	};







	struct Extra_Context {
		Handle _prev;
		Handle _next;
	};



	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;
		using BASE::_next;
		using BASE::_prev;

		friend Binary_Tree;

	public:
		FORWARDING_CONSTRUCTOR(Accessor,BASE) { BASE::iterator()._init(); }

	public:
		auto& key()       { return _container()[_handle()].key; }
		auto& key() const { return _container()[_handle()].key; }

		auto& val()       { return _container()[_handle()].val; }
		auto& val() const { return _container()[_handle()].val; }

	public:
		bool exists() { return _handle().valid() && _handle() != _container().super_root; }

		auto left()        { return Accessor<C    >( &_container(), _node().left ); }
		auto left()  const { return Accessor<CONST>( &_container(), _node().left ); }

		auto right()       { return Accessor<C    >( &_container(), _node().right ); }
		auto right() const { return Accessor<CONST>( &_container(), _node().right ); }

		auto parent()       { return Accessor<C    >( &_container(), _node().parent ); }
		auto parent() const { return Accessor<CONST>( &_container(), _node().parent ); }

		bool has_left()   const { return _node().left.valid(); }
		bool has_right()  const { return _node().right.valid(); }
		bool has_parent() const { return _node().parent.valid(); }

		bool is_left() const {
			auto parent = _node().parent;
			DCHECK(parent.valid());
			return _handle() == _node(parent).left;
		}

		bool is_right() const {
			auto parent = _node().parent;
			DCHECK(parent.valid());
			return _handle() == _node(parent).right;
		}

		template<class... Args>
		auto emplace_left(Args&&... args) {
			static_assert(C == MUTAB, "called construct_right() on CONST accessor");
			DCHECK( !left().exists() );
			auto new_node = _container()._alloc().construct();
			new_node().construct( std::forward<Args>(args)... );
			link_left(new_node);
			return left();
		}

		template<class... Args>
		auto emplace_right(Args&&... args) {
			static_assert(C == MUTAB, "called construct_right() on CONST accessor");
			DCHECK( !right().exists() );
			auto new_node = _container()._alloc().construct();
			new_node().construct( std::forward<Args>(args)... );
			link_right(new_node);
			return right();
		}

		void link_left(Handle new_left) {
			static_assert(C == MUTAB, "called link_left() on CONST accessor");
			DCHECK( !left().exists() );
			_container()._alloc()[_handle()].left = new_left;
			_container()._alloc()[new_left].parent = _handle();
		}

		void link_right(Handle new_right) {
			static_assert(C == MUTAB, "called link_right() on CONST accessor");
			DCHECK( !right().exists() );
			_container()._alloc()[_handle()].right = new_right;
			_container()._alloc()[new_right].parent = _handle();
		}

		void unlink_left() {
			static_assert(C == MUTAB, "called unlink_left() on CONST accessor");
			auto& l = _container()._alloc()[_handle()].left;
			_container()._alloc()[l].parent.reset();
			l.reset();
		}

		void unlink_right() {
			static_assert(C == MUTAB, "called unlink_right() on CONST accessor");
			auto& r = _container()._alloc()[_handle()].right;
			_container()._alloc()[r].parent.reset();
			r.reset();
		}

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			DCHECK_NE(_handle(), _container().super_root) << "called erase() on super_root";
			DCHECK(exists());
			DCHECK(!_node().parent.valid());
			DCHECK(!_node().left.valid());
			DCHECK(!_node().right.valid());

			_container()._alloc()[_handle()].destruct();
			_container()._alloc().destruct( _handle() );
		}

		void unlink_and_erase() {
			static_assert(C == MUTAB, "called unlink_and_erase() on CONST accessor");
			DCHECK_NE(_handle(), _container().super_root) << "called unlink_and_erase() on super_root";
			DCHECK(exists());

			auto& node = _container()._alloc()[_handle()];
			if(node.parent.valid()) {
				auto& par = _container().alloc()[node.parent];
				DCHECK(par.left == _handle() || par.right == _handle());
				if(par.left  == _handle()) par.left.reset();
				if(par.right == _handle()) par.right.reset();
			}

			if(node.left .valid()) _container()._alloc()[node.left ].parent.reset();
			if(node.right.valid()) _container()._alloc()[node.right].parent.reset();

			erase();
		}

		// remove node and replace the hole with _next
		void bst_erase() {
			static_assert(C == MUTAB, "called bst_erase() on CONST accessor");
			if( _cut_out(_handle(), true) ) return;
			
			// replace with _next
			bool r = _cut_out(_next, false);
			DCHECK(r);
			_node(_next).left = _node().left;
			_node(_next).right = _node().right;
			_node(_next).parent = _node().parent;
			if(is_left()) _node(_node().parent).left = _next;
			else          _node(_node().parent).right = _next;

			_container()._alloc()[_handle()].destruct();
			_container()._alloc().destruct( _handle() );
		}

	private:
		// try to cut out node (possible if max 1 child)
		bool _cut_out(Handle h, bool destruct) {
			if(!_node(h).right.valid()) {
				if(_node(h).left.valid()) {
					_node(_node(h).left).parent = _node(h).parent;
				}

				if(_node(_node(h).parent).left == h) _node(_node(h).parent).left  = _node(h).left;
				else                                 _node(_node(h).parent).right = _node(h).left;
			}
			else if(!_node(h).left.valid()) {
				if(_node(h).right.valid()) {
					_node(_node(h).right).parent = _node(h).parent;
				}

				if(_node(_node(h).parent).left == h) _node(_node(h).parent).left  = _node(h).right;
				else                                 _node(_node(h).parent).right = _node(h).right;
			}
			else {
				return false;
			}

			if(destruct) {
				_node(h).destruct();
				_container()._alloc().destruct( h );
			}
			return true;
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
		using BASE::_container;
		using BASE::_handle;
		using BASE::_next;
		using BASE::_prev;

	public:
		FORWARDING_CONSTRUCTOR(Iterator, BASE) { _init(); }

	private:
		friend BASE;

		void _increment() {
			_prev = _handle();
			_handle() = _next;
			DCHECK( _handle().valid() ) << "followed broken tree link";
			_update_next();
		}

		void _decrement() {
			_next = _handle;
			_handle = _prev;
			DCHECK( _handle().valid() ) << "followed broken tree link";
			_update_prev();
		}


	private:
		void _update_prev() {
			DCHECK(_handle().valid());

			auto handle = _handle();
			if(_node(handle).left.valid()) {
				// go 1 left and n right
				handle = _node(handle).left;
				while(_node(handle).right.valid()) handle = _node(handle).right;
				_prev = handle;
			}
			else {
				// go n up-right and 1 up-left
				for(;;) {
					Handle parent = _node(handle).parent;
					DCHECK(parent.valid());
					if(_node(parent).right == handle) {
						_prev = parent;
						return;
					}
					handle = parent;
				}
			}
		}

		void _update_next() {
			DCHECK(_handle().valid());

			auto handle = _handle();
			if(_node(handle).right.valid()) {
				// go 1 right and n left
				handle = _node(handle).right;
				while(_node(handle).left.valid()) handle = _node(handle).left;
				_next = handle;
			}
			else {
				// go n up-left and 1 up-right
				for(;;) {
					Handle parent = _node(handle).parent;
					DCHECK(parent.valid());
					if(_node(parent).left == handle) {
						_next = parent;
						return;
					}
					handle = parent;
				}
			}
		}

		void _init() {
			_update_prev();
			_update_next();
		}
		friend Accessor<C>;

	private:
		auto& _node()       { return _container()._alloc()[_handle()]; }
		auto& _node() const { return _container()._alloc()[_handle()]; }
		auto& _node(Handle h)       { return _container()._alloc()[h]; }
		auto& _node(Handle h) const { return _container()._alloc()[h]; }
	};












	class Binary_Tree : private Allocator {
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
		Binary_Tree() {
			super_root = _alloc().construct().handle();
		}

		~Binary_Tree() {
			for(auto& e : *this) {
				_alloc()[e._handle()].destruct();
				_alloc().destruct(e._handle());
			}
			_alloc().destruct(super_root);
		}

		bool empty() const {
			auto& r = _alloc()[super_root];
			DCHECK_EQ(r.left, r.right);
			return !r.left.valid();
		}

		auto root()       { DCHECK(!empty()); return Accessor<MUTAB>(this, _alloc()[super_root].right); }
		auto root() const { DCHECK(!empty()); return Accessor<CONST>(this, _alloc()[super_root].right); }

		template<class... Args>
		auto emplace_root(Args&&... args) {
			// remove old root
			//if(sr.left.valid()) {
			//	DCHECK_EQ(sr.left, sr.right);
			//	_alloc()[sr.left].parent.reset();
			//}
			auto& sr = _alloc()[super_root];
			auto new_node = _alloc().construct();
			new_node().construct( std::forward<Args>(args)... );
			sr.left = sr.right = new_node.handle();
			new_node().parent = super_root;
			return Accessor<MUTAB>(this, new_node);
		}

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

	public:
		auto& operator[](Handle handle)       { return _alloc()[handle].get()(); }
		auto& operator[](Handle handle) const { return _alloc()[handle].get()(); }

		auto operator()(Handle handle)       { return Accessor<MUTAB>(this, handle); }
		auto operator()(Handle handle) const { return Accessor<CONST>(this, handle); }

	private:
		Handle super_root;

	public:
		auto before_begin()       { return Iterator<MUTAB>(this, super_root); }
		auto before_begin() const { return Iterator<CONST>(this, super_root); }

		auto begin()       { auto it = before_begin(); if(!empty()) ++it; return it; }
		auto begin() const { auto it = before_begin(); if(!empty()) ++it; return it; }

		auto end()       { return Iterator<MUTAB>(this, super_root); }
		auto end() const { return Iterator<CONST>(this, super_root); }
	};



	struct With_Builder : Binary_Tree {
		FORWARDING_CONSTRUCTOR(With_Builder, Binary_Tree) {}

		template<class NEW_ALLOCATOR>
		using ALLOCATOR = typename Context<Key, Val, Aggreg, Propag,
			Path_Parents, Pp_Aggreg, Pp_Propag, NEW_ALLOCATOR> ::With_Builder;
	};


}; // struct Context


} // binary_tree
} // internal






template<class KEY, class VAL = typename internal::binary_tree...?>
using Binary_Tree = typename internal::binary_tree::Context<
	KEY,
	VAL,
	void, // aggreg
	void, // propag
	false, // path parents
	void, // pp_aggreg
	void, // pp_propag
	Random_Allocator<int> // will be rebound anyway
> :: With_Builder;







} // salgo