#include <salgo/splay-tree.hpp>
#include <salgo/crude-allocator.hpp>

#include <gtest/gtest.h>

using namespace salgo;




TEST(Splay_Tree, initializer_list) {
	Splay_Tree<int> tree = {9,3,7,2,3,1,5,7};
	std::vector<int> v;
	for(auto& e : tree) v.emplace_back(e);
	EXPECT_EQ(std::vector<int>({1,2,3,3,5,7,7,9}), v);
}


TEST(Splay_Tree, with_vals) {
	Splay_Tree<int,int> tree = {{55,5}, {22,2}, {77,7}, {44,4}, {11,1}, {33,3}};
	std::vector<int> v;
	for(auto& e : tree) v.emplace_back( e.val() );
	EXPECT_EQ(std::vector<int>({1,2,3,4,5,7}), v);
}




namespace {
	int g_constructors = 0;
	int g_destructors = 0;
}

TEST(Splay_Tree, destructors) {
	struct S {
		S(int) { ++g_constructors; }
		~S() { ++g_destructors; }
		S(const S&) { ++g_constructors; }
		bool operator<(const S&) const { return true; }
	};
	g_constructors = 0;
	g_destructors = 0;

	{
		Splay_Tree<S> ::ALLOCATOR<Crude_Allocator<int>> tree = {1,2,3,4,5};
	}
	EXPECT_EQ(g_constructors, g_destructors);
	
	{
		Splay_Tree<S,S> ::ALLOCATOR<Crude_Allocator<int>> tree = {{1,1},{2,2}};
	}
	EXPECT_EQ(g_constructors, g_destructors);
}


