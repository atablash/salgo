#pragma once

#include "binary-tree.hpp"


namespace salgo {
namespace internal {
namespace link_cut_tree {





//
// add VAL
//
template<bool, class T> struct Add_val {
	typename salgo::Inplace_Storage<T> ::PERSISTENT val;

	template<class... Args>
	void construct(Args&&... args) { val.construct( std::forward<Args>(args)... ); }

	void destruct() { val.destruct(); }
};
template<class T> struct Add_val<false,T> {
	void construct(...) {}
	void destruct() {}
};






template<class _VAL, class _ALLOCATOR>
struct Context {


	using Val = _VAL;
	static constexpr bool Has_Val = !std::is_same_v<Val,void>;



	struct Node;
	using Binary_Tree = salgo::Binary_Tree<Node> ::ALLOCATOR<_ALLOCATOR>;

	using       Handle = Binary_Tree::      Handle;
	using Handle_Small = Binary_Tree::Handle_Small;



	struct Node : Add_val<Has_Val, Val> {
		Handle_Small path_parent;
	};







	template<Const_Flag C>
	class Accessor : public Accessor_Base<> {

	};


	template<Const_Flag C>
	class Iterator : public Iterator_Base<> {

	};






	class Link_Cut_Tree {
	public:
		void access(Handle v) {
			splay()
		}

		void link(Handle v, Handle new_parent) {}
		void cut(Handle v) {}

		auto lca(Handle a, Handle b) {}

		auto find_root(Handle v) {}

	private:
		Binary_Tree<Node> _tree;
	};





}; // struct Context
} // namespace link_cut_tree
} // namespace internal











} // namespace salgo

