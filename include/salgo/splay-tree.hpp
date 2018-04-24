#pragma once


#include "binary-tree.hpp"
#include "bst.hpp"


namespace salgo {


namespace internal {
namespace splay_tree {



template<class _KEY, class _VAL, class _ALLOCATOR>
struct Context {

	using Key = _KEY;
	using Val = _VAL;

	using Key_Val = salgo::Key_Val<Key,Val>;

	using Tree = salgo::Binary_Tree<Key_Val> ::ALLOCATOR<_ALLOCATOR>;

	class Splay_Tree;
	using Container = Splay_Tree;



	using       Handle = Tree::      Handle;
	using Small_Handle = Tree::Small_Handle;




	template<Const_Flag C>
	struct Extra_Context {
		Handle _prev;
		Handle _next;
	};



	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

		friend Splay_Tree;

	public:
		FORWARDING_CONSTRUCTOR(Accessor,BASE) { BASE::iterator()._init(); }
	};





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
			_prev = bst_prev(_container->_tree, _handle);
		}

		void _update_next() {
			_next = bst_next(_container->_tree, _handle);
		}

		void _init() {
			_update_prev();
			_update_next();
		}
		friend Accessor<C>;
	};






	class Splay_Tree {
	public:
		template<class... KV>
		auto emplace(KV&&... kv) { return Accessor<MUTAB>(this, _tree.bst_emplace( std::forward<KV>(kv)... )); }

	private:
		Tree _tree;
	};



}; // struct Context




} // splay_tree
} // namespace internal








} // namespace salgo
