#include <salgo/union-find.hpp>

#include <gtest/gtest.h>


using namespace salgo;




TEST(Union_Find, simple) {
	Union_Find uf;
	auto v0 = uf.add();
	auto v1 = uf.add();

	EXPECT_NE( v0, v1 );

	uf.merge(v0, v1);
	EXPECT_EQ( v0, v1 );
	EXPECT_EQ( uf(v0), uf(v1) );
}


TEST(Union_Find, construct_n) {
	Union_Find ::COUNTABLE uf(10);
	EXPECT_EQ(10, uf.domain());
	EXPECT_EQ(10, uf.count());

	EXPECT_NE(uf(2), uf(5));

	uf(2).merge_with(5);
	EXPECT_EQ(uf(2), uf(5));
}


TEST(Union_Find, data_int) {
	Union_Find ::DATA<int> uf(10, 33);

	uf(2).merge_with(5);
	EXPECT_EQ( 33*2, uf[2] );
	EXPECT_EQ( 33*2, uf[5] );
}

TEST(Union_Find, data_custom) {
	struct S {
		int val = 1;
		void merge_with(S& o) const { o.val *= val; }
	};

	Union_Find ::DATA<S> uf(2);
	uf[0].val = 3;
	uf[1].val = 5;

	uf(0).merge_with(1);
	EXPECT_EQ( 3*5, uf[0].val );
	EXPECT_EQ( 3*5, uf[1].val );
}


int g_constructed = 0;
TEST(Union_Find, data_destruct_and_count) {
	struct S {
		S()    { ++g_constructed; }
		S(S&&) { ++g_constructed; }
		~S()   { --g_constructed; }
		void merge_with(const S&) {}
	};

	{
		Union_Find::DATA<S>::COUNTABLE uf(10);
		EXPECT_EQ(10, uf.count());
		EXPECT_EQ(10, g_constructed);

		uf.add();
		uf.add();
		EXPECT_EQ(12, uf.count());
		EXPECT_EQ(12, g_constructed);

		uf(2).merge_with(5);
		uf(3).merge_with(11);
		EXPECT_EQ(10, uf.count());
		EXPECT_EQ(10, g_constructed);
	}

	EXPECT_EQ(0, g_constructed);
}


