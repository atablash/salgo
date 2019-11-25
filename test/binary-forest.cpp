#include <salgo/binary-forest>
#include <salgo/inorder>

#include <gtest/gtest.h>

using namespace salgo;


//                  |
//         o        |
//        / \       |
//       o   o      |
//      /   / \     |
//     o   o   o    |
//            /     |
//           o      |
//                  |
template<class TREE>
auto sample_tree_1(TREE& tree) {
	auto root = tree.emplace(33);
	EXPECT_TRUE( root.handle() >= 0 );

	root.emplace_left(22);
	root.left().emplace_left(11);

	root.emplace_right(55);
	root.right().emplace_left(44);
	root.right().emplace_right(77);

	root.right().right().emplace_left(66);

	EXPECT_TRUE( root.handle() >= 0 );
	return root;
}


using My_Binary_Forest = N_Ary_Forest<2> // AKA Binary_Forest
	::PARENT_LINKS :: CHILD_LINKS;


TEST(Binary_Forest, no_val) {
	My_Binary_Forest tree;
	auto root = tree.emplace();
	root.emplace_left();
	root.emplace_right();
}


TEST(Binary_Forest, iteration) {
	My_Binary_Forest ::DATA<int> tree;
	sample_tree_1(tree);

	std::multiset<int> vals;
	for(auto& e : tree) vals.emplace(e());
	EXPECT_EQ(vals, std::multiset<int>({11,22,33,44,55,66,77}));
}


namespace {
	int g_constructors = 0;
	int g_destructors = 0;
}

TEST(Binary_Forest, destructors) {
	struct S {
		S(int) { ++g_constructors; }
		S(const S&&) { ++g_constructors; }
		~S() { ++g_destructors; }
		S(const S&) = delete;
	};
	g_constructors = 0;
	g_destructors = 0;

	{
		My_Binary_Forest ::DATA<S> tree;
		sample_tree_1(tree);
	}
	EXPECT_EQ(g_constructors, g_destructors);
}



TEST(Binary_Forest, traverse_and_erase) {
	My_Binary_Forest ::DATA<int> tree;
	auto v = sample_tree_1( tree );

	v = v.left().left();
	v.unlink_and_erase();

	v = v.parent();
	v.unlink_and_erase();

	v = v.parent();
	v.unlink_and_erase();

	v = v.right().left();
	v.unlink_and_erase();

	v = v.parent();
	v.unlink_and_erase();

	v = v.right();
	v.unlink_and_erase();

	v = v.left();
	v.erase();

	EXPECT_TRUE( tree.empty() );
}



TEST(Binary_Forest, inorder) {
	My_Binary_Forest ::DATA<int> tree;
	auto root = sample_tree_1( tree );
	EXPECT_TRUE( root.handle().valid() );

	std::vector<int> vals;
	for(auto& e : Inorder(root)) {
		vals.emplace_back( e );
	}
	EXPECT_EQ(vals, std::vector<int>({11,22,33,44,55,66,77}));
}


/* TODO
TEST(Binary_Forest, inorder_erase) {
	Binary_Forest<int> tree;
	auto root = sample_tree_1( tree );
	EXPECT_TRUE( root.handle().valid() );

	for(auto& e : Inorder(root)) {
		LOG(INFO) << "visiting " << e() << " with handle " << e.handle();
		if(e % 2) {
			LOG(INFO) << "erasing";
			e.erase();
		}
	}

	std::vector<int> vals;
	for(auto& e : Inorder(root)) {
		vals.emplace_back( e );
	}
	EXPECT_EQ(vals, std::vector<int>({11,22,33,44,55,66,77}));
}
*/

