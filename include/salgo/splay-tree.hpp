#pragma once


#include "binary-tree.hpp"


namespace salgo {


namespace internal {
namespace splay_tree {



template<class _KEY, class _VAL, class _ALLOCATOR>
struct Context {

	using Key = _KEY;
	using Val = _VAL;

	using Tree = typename salgo::Binary_Tree<Key,Val> ::template ALLOCATOR<_ALLOCATOR>;
	using Key_Val = typename Tree::Key_Val;

	class Splay_Tree;
	using Container = Splay_Tree;



	using       Handle = typename Tree::      Handle;
	using Small_Handle = typename Tree::Small_Handle;




	template<Const_Flag C>
	struct Reference {
		typename Tree::template Iterator<C> _iter;

		template<class ITER>
		Reference(ITER&& iter) : _iter( std::forward<ITER>(iter) ) {}

	protected:
		auto& _val()       { return _iter(); }
		auto& _val() const { return _iter(); }
		auto _handle() const { return _iter.handle(); }
	};



	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::_iter;

		friend Splay_Tree;

	public:
		auto& key()       { return _iter->key(); }
		auto& key() const { return _iter->key(); }

		auto& val()       { return _iter->val(); }
		auto& val() const { return _iter->val(); }

	public:
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
	};





	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		using BASE::_iter;

	public:
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;
		void _increment() { ++_iter; }
		void _decrement() { --_iter; }
		auto& _get_comparable() const { return _iter; }
	};






	class Splay_Tree {
	public:
		Splay_Tree(std::initializer_list<Key_Val>&& il) {
			for(auto&& e : il) emplace(e);
		}

	public:
		template<class... KV>
		auto emplace(KV&&... kv) { return Accessor<MUTAB>( _tree.bst_emplace( std::forward<KV>(kv)... ).iterator() ); }

	public:
		auto before_begin()       { return Iterator<MUTAB>( _tree.before_begin() ); }
		auto before_begin() const { return Iterator<CONST>( _tree.before_begin() ); }

		auto begin()       { return Iterator<MUTAB>( _tree.begin() ); }
		auto begin() const { return Iterator<CONST>( _tree.begin() ); }

		auto end()       { return Iterator<MUTAB>( _tree.end() ); }
		auto end() const { return Iterator<CONST>( _tree.end() ); }

	private:
		Tree _tree;
	};




	struct With_Builder : Splay_Tree {
		FORWARDING_CONSTRUCTOR(With_Builder, Splay_Tree) {}
		With_Builder(std::initializer_list<Key_Val>&& il) : Splay_Tree( std::move(il) ) {}
	};



}; // struct Context




} // splay_tree
} // namespace internal





template<class KEY, class VAL=void>
using Splay_Tree = typename internal::splay_tree::Context<
	KEY,
	VAL,
	Random_Allocator<int> // will be rebound anyway
> :: With_Builder;






} // namespace salgo
