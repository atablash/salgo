#include <salgo/binary-tree.hpp>

#include <gtest/gtest.h>

using namespace salgo;


//
//         o
//        / \
//       o   o
//      /   / \
//     o   o   o
//            /
//           o
//
template<class TREE>
void sample_tree_1(TREE& tree) {
	auto root = tree.construct_root(33);
	root.construct_left(22);
	root.left().construct_left(11);

	root.construct_right(55);
	root.right().construct_left(44);
	root.right().construct_right(77);

	root.right().right().construct_left(66);
}



TEST(Binary_Tree, no_val) {
	Binary_Tree<void> tree;
	tree.construct_root();
	tree.root().construct_right();
}


TEST(Binary_Tree, iteration) {
	Binary_Tree<int> tree;
	sample_tree_1(tree);
	std::vector<int> vals;
	for(auto& e : tree) vals.emplace_back(e);
	DCHECK(vals == std::vector<int>({11,22,33,44,55,66,77}));
}


namespace {
	int g_constructors = 0;
	int g_destructors = 0;
}

TEST(Binary_Tree, destructors) {
	struct S {
		S(int) { ++g_constructors; }
		~S() { ++g_destructors; }
		S(const S&) = delete;
	};
	g_constructors = 0;
	g_destructors = 0;

	{
		Binary_Tree<S> tree;
		sample_tree_1(tree);
	}
	DCHECK_EQ(g_constructors, g_destructors);
}
