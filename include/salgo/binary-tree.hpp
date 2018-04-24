#pragma once

#include "stack-storage.hpp"
#include "const-flag.hpp"
#include "accessors.hpp"
#include "random-allocator.hpp"


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
// add VAL
//
template<bool, class T> struct Add_val {
	typename salgo::Stack_Storage<T> ::PERSISTENT val;

	template<class... Args>
	void construct(Args&&... args) { val.construct( std::forward<Args>(args)... ); }

	void destruct() { val.destruct(); }
};
template<class T> struct Add_val<false,T> {
	void construct(...) {}
	void destruct() {}
};




//
// add AGGREG
//
template<bool, class T> struct Add_aggreg {
	typename salgo::Stack_Storage<T> ::PERSISTENT aggreg;

	template<class... Args>
	void construct(Args&&... args) { aggreg.construct( std::forward<Args>(args)... ); }

	void destruct() { aggreg.destruct(); }
};
template<class T> struct Add_aggreg<false,T> {
	void construct(...) {}
	void destruct() {}
};



//
// add PROPAG
//
template<bool, class T> struct Add_propag {
	typename salgo::Stack_Storage<T> ::PERSISTENT propag;

	template<class... Args>
	void construct(Args&&... args) { propag.construct( std::forward<Args>(args)... ); }

	void destruct() { propag.destruct(); }
};
template<class T> struct Add_propag<false,T> {
	void construct(...) {}
	void destruct() {}
};







template<class _VAL, class _AGGREG, class _PROPAG, bool _PATH_PARENTS, class _PP_AGGREG, class _PP_PROPAG, class _ALLOCATOR>
struct Context {
	using Val = _VAL;
	static constexpr bool Has_Val    = !std::is_same_v<Val,   void>;

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



	struct Node :
			Add_val<Has_Val, Val>,
			Add_aggreg<Has_Aggreg, Aggreg>,
			Add_propag<Has_Propag, Propag> {

		template<class... Args>
		void construct(Args&&... args) {
			Add_val<Has_Val, Val>::construct( std::forward<Args>(args)... );
			Add_aggreg<Has_Aggreg, Aggreg>::construct();
			Add_propag<Has_Propag, Propag>::construct();
		}

		void destruct() {
			Add_val<Has_Val, Val>::destruct();
			Add_aggreg<Has_Aggreg, Aggreg>::destruct();
			Add_propag<Has_Propag, Propag>::destruct();
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
		bool exists() { return _handle.valid() && _handle != _container->super_root; }

		auto left()        { return Accessor<C    >(_container, _node().left); }
		auto left()  const { return Accessor<CONST>(_container, _node().left); }

		auto right()       { return Accessor<C    >(_container, _node().right); }
		auto right() const { return Accessor<CONST>(_container, _node().right); }

		auto parent()       { return Accessor<C    >(_container, _node().parent); }
		auto parent() const { return Accessor<CONST>(_container, _node().parent); }

		bool has_left()   const { return _node().left.valid(); }
		bool has_right()  const { return _node().right.valid(); }
		bool has_parent() const { return _node().parent.valid(); }

		bool is_left() const {
			auto parent = _node().parent;
			DCHECK(parent.valid());
			return _handle == _node(parent).left;
		}

		bool is_right() const {
			auto parent = _node().parent;
			DCHECK(parent.valid());
			return _handle == _node(parent).right;
		}

		template<class... Args>
		auto construct_left(Args&&... args) {
			static_assert(C == MUTAB, "called construct_right() on CONST accessor");
			DCHECK( !left().exists() );
			auto new_node = _container->_alloc().construct();
			new_node().construct( std::forward<Args>(args)... );
			link_left(new_node);
			return left();
		}

		template<class... Args>
		auto construct_right(Args&&... args) {
			static_assert(C == MUTAB, "called construct_right() on CONST accessor");
			DCHECK( !right().exists() );
			auto new_node = _container->_alloc().construct();
			new_node().construct( std::forward<Args>(args)... );
			link_right(new_node);
			return right();
		}

		void link_left(Handle new_left) {
			static_assert(C == MUTAB, "called link_left() on CONST accessor");
			DCHECK( !left().exists() );
			_container->_alloc()[_handle].left = new_left;
			_container->_alloc()[new_left].parent = _handle;
		}

		void link_right(Handle new_right) {
			static_assert(C == MUTAB, "called link_right() on CONST accessor");
			DCHECK( !right().exists() );
			_container->_alloc()[_handle].right = new_right;
			_container->_alloc()[new_right].parent = _handle;
		}

		void unlink_left() {
			static_assert(C == MUTAB, "called unlink_left() on CONST accessor");
			auto& l = _container->_alloc()[_handle].left;
			_container->_alloc()[l].parent.reset();
			l.reset();
		}

		void unlink_right() {
			static_assert(C == MUTAB, "called unlink_right() on CONST accessor");
			auto& r = _container->_alloc()[_handle].right;
			_container->_alloc()[r].parent.reset();
			r.reset();
		}

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			DCHECK_NE(_handle, _container->super_root) << "called erase() on super_root";
			DCHECK(exists());
			DCHECK(!_node().parent.valid());
			DCHECK(!_node().left.valid());
			DCHECK(!_node().right.valid());

			_container->_alloc()[_handle].destruct();
			_container->_alloc().destruct( _handle );
		}

		void unlink_and_erase() {
			static_assert(C == MUTAB, "called unlink_and_erase() on CONST accessor");
			DCHECK_NE(_handle, _container->super_root) << "called unlink_and_erase() on super_root";
			DCHECK(exists());

			auto& node = _container->_alloc()[_handle];
			if(node.parent.valid()) {
				auto& par = _container->alloc()[node.parent];
				DCHECK(par.left == _handle || par.right == _handle);
				if(par.left  == _handle) par.left.reset();
				if(par.right == _handle) par.right.reset();
			}

			if(node.left .valid()) _container->_alloc()[node.left ].parent.reset();
			if(node.right.valid()) _container->_alloc()[node.right].parent.reset();

			erase();
		}

		//
		//
		//
		void bst_erase() {
			static_assert(C == MUTAB, "called bst_erase() on CONST accessor");
			if( _cut_out(_handle, true) ) return;
			
			// replace with _next
			bool r = _cut_out(_next, false);
			DCHECK(r);
			_node(_next).left = _node().left;
			_node(_next).right = _node().right;
			_node(_next).parent = _node().parent;
			if(is_left()) _node(_node().parent).left = _next;
			else          _node(_node().parent).right = _next;

			_container->_alloc()[_handle].destruct();
			_container->_alloc().destruct( _handle );
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
				_container->_alloc()[h].destruct();
				_container->_alloc().destruct( h );
			}
			return true;
		}

		//void aggregate() {
		//	static_assert(C == MUTAB, "called aggregate() on CONST accessor");
		//	_node().aggreg = Aggreg();
		//	if() _node().aggreg.aggregate();
		//}

	private:
		auto& _node(Handle h = _handle)       { return _container->_alloc()[h]; }
		auto& _node(Handle h = _handle) const { return _container->_alloc()[h]; }
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
			_prev = _handle;
			_handle = _next;
			DCHECK( _handle.valid() ) << "followed broken tree link";
			_update_next();
		}

		void _decrement() {
			_next = _handle;
			_handle = _prev;
			DCHECK( _handle.valid() ) << "followed broken tree link";
			_update_prev();
		}


	private:
		void _update_prev() {
			_prev = bst_prev(_container->_alloc(), _handle);
		}

		void _update_next() {
			_next = bst_next(_container->_alloc(), _handle);
		}

		void _init() {
			_update_prev();
			_update_next();
		}
		friend Accessor<C>;
	};












	class Binary_Tree : private Allocator {
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;
		friend Iterator<MUTAB>;
		friend Iterator<CONST>;

	private:
		auto& _alloc()       { return *static_cast<      Allocator*>(this); }
		auto& _alloc() const { return *static_cast<const Allocator*>(this); }

	public:
		Binary_Tree() {
			super_root = _alloc().construct().handle();
		}

		~Binary_Tree() {
			for(auto& e : *this) {
				_alloc()[e._handle].destruct();
				_alloc().destruct(e._handle);
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
		auto construct_root(Args&&... args) {
			// remove old root
			auto& sr = _alloc()[super_root];
			if(sr.left.valid()) {
				DCHECK_EQ(sr.left, sr.right);
				_alloc()[sr.left].parent.reset();
			}
			auto new_node = _alloc().construct();
			new_node().construct( std::forward<Args>(args)... );
			sr.left = sr.right = new_node.handle();
			new_node().parent = super_root;
			return Accessor<MUTAB>(this, new_node);
		}

	public:
		auto& operator[](Handle handle)       { return _alloc()[handle].val.get(); }
		auto& operator[](Handle handle) const { return _alloc()[handle].val.get(); }

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
	};


}; // struct Context


} // binary_tree
} // internal






template<class VAL>
using Binary_Tree = typename internal::binary_tree::Context<
	VAL,
	void,
	void,
	false, // path parents
	void, // pp_aggreg
	void, // pp_propag
	Random_Allocator<char> // will be rebound anyway
> :: With_Builder;







} // salgo