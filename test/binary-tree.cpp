#include <salgo/binary-tree.hpp>
#include <salgo/crude-allocator.hpp>

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
	auto root = tree.emplace_root(33);
	root.emplace_left(22);
	root.left().emplace_left(11);

	root.emplace_right(55);
	root.right().emplace_left(44);
	root.right().emplace_right(77);

	root.right().right().emplace_left(66);
}



TEST(Binary_Tree, no_val) {
	Binary_Tree<void,void> tree;
	tree.emplace_root();
	tree.root().emplace_right();
}


TEST(Binary_Tree, iteration) {
	Binary_Tree<void,int> tree;
	sample_tree_1(tree);
	std::vector<int> vals;
	for(auto& e : tree) vals.emplace_back(e);
	EXPECT_EQ(vals, std::vector<int>({11,22,33,44,55,66,77}));
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
		Binary_Tree<void,S> ::ALLOCATOR<Crude_Allocator<int>> tree;
		sample_tree_1(tree);
	}
	EXPECT_EQ(g_constructors, g_destructors);

	{
		Binary_Tree<S,void> ::ALLOCATOR<Crude_Allocator<int>> tree;
		sample_tree_1(tree);
	}
	EXPECT_EQ(g_constructors, g_destructors);
}
