#include <salgo/rooted-forest.hpp>
#include <salgo/inorder.hpp>

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
	auto root = tree.emplace(33);
	root.emplace_left(22);
	root.left().emplace_left(11);

	root.emplace_right(55);
	root.right().emplace_left(44);
	root.right().emplace_right(77);

	root.right().right().emplace_left(66);
}



TEST(Rooted_Forest, no_val) {
	Rooted_Forest<2> tree;
	auto root = tree.emplace();
	root.emplace_left();
	root.emplace_right();
}


TEST(Rooted_Forest, iteration) {
	Rooted_Forest<2,int> tree;
	sample_tree_1(tree);

	std::multiset<int> vals;
	for(auto& e : tree) vals.emplace(e());
	EXPECT_EQ(vals, std::multiset<int>({11,22,33,44,55,66,77}));
}


namespace {
	int g_constructors = 0;
	int g_destructors = 0;
}

TEST(Rooted_Forest, destructors) {
	struct S {
		S(int) { ++g_constructors; }
		S(const S&&) { ++g_constructors; }
		~S() { ++g_destructors; }
		S(const S&) = delete;
	};
	g_constructors = 0;
	g_destructors = 0;

	{
		Binary_Forest<void,S> tree;
		sample_tree_1(tree);
	}
	EXPECT_EQ(g_constructors, g_destructors);

	{
		Binary_Forest<S,void> tree;
		sample_tree_1(tree);
	}
	EXPECT_EQ(g_constructors, g_destructors);
}



TEST(Rooted_Forest, inorder) {
	Binary_Forest<int> tree;
	sample_tree_1( tree );

	std::vector<int> vals;
	for(auto& e : Inorder(tree)) {
		vals.emplace_back( e );
	}
	EXPECT_EQ(vals, std::vector<int>({11,22,33,44,55,66,77}));
}

