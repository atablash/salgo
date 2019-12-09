#include <salgo/dynamic-array>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;







TEST(Sparse_array, not_iterable) {
	Dynamic_Array<int>::SPARSE m;
	m.emplace_back(1);
	m.emplace_back(2);
	m.emplace_back(3);
	m.emplace_back(4);
	m.emplace_back(5);

	EXPECT_EQ(5, m.domain());

	EXPECT_EQ(1, m[0]);
	EXPECT_EQ(2, m(1)());
	EXPECT_EQ(3, m[2]);
	EXPECT_EQ(4, m[3]);
	EXPECT_EQ(5, m[4]);

	m(0).destruct();
	m(1).destruct();
	m(2).destruct();
	m(3).destruct();
	m(4).destruct();
}






/////////////////////////////////////////
// this is currently illegal (allows memory leaks)
/////////////////////////////////////////

// TEST(Sparse_array, not_iterable_nontrivial) {
// 	struct A {
// 		int val;
// 		int* counter;

// 		A(int v, int* p) : val(v), counter(p) {}
// 		~A() { ++*counter; }
// 	};

// 	Dynamic_Array< unique_ptr<A> >::SPARSE m(3);
// 	// m.emplace_back( make_unique<int>(1) ); // should not compile

// 	EXPECT_EQ(3, m.domain());
// 	EXPECT_EQ(3, m.capacity());

// 	int counter = 0;

// 	m[0] = make_unique<A>(1,&counter);
// 	m[1] = make_unique<A>(2,&counter);
// 	m[2] = make_unique<A>(3,&counter);

// 	EXPECT_EQ(1, m(0)()->val);
// 	EXPECT_EQ(2, m[1]->val);
// 	EXPECT_EQ(3, m(2)()->val);

// 	m(0).destruct();
// 	m(1).destruct();
// 	m(2).destruct();

// 	EXPECT_EQ(3, counter);
// }










template<class MB>
static void push_delete_compact_common(MB& m) {
	m.emplace_back(1); //
	m.emplace_back(2);
	m.emplace_back(3); //
	m.emplace_back(4);
	m.emplace_back(5); //

	EXPECT_EQ(5, m.count());

	//EXPECT_EQ(0, m.domain_begin());
	//EXPECT_EQ(5, m.domain_end());

	EXPECT_EQ(5, m.domain());

	EXPECT_EQ(1, m(0)());
	EXPECT_EQ(2, m(1)());
	EXPECT_EQ(3, m(2)());
	EXPECT_EQ(4, m(3)());
	EXPECT_EQ(5, m(4)());

	{
		int sum = 0;
		for(const auto& e : m) sum += e();
		EXPECT_EQ(15, sum);
	}

	m(0).destruct();
	m(2).destruct();
	m(4).destruct();

	EXPECT_TRUE(  m(1).is_constructed() );
	EXPECT_TRUE( m(2).is_not_constructed() );

	{
		int sum = 0;
		for(const auto& e : m) sum += e();
		EXPECT_EQ(6, sum);
	}

	{
		vector<pair<int,int>> remap;
		m.compact([&](int fr, int to){ remap.emplace_back(fr,to); });
		EXPECT_EQ( decltype(remap)({{1,0}, {3,1}}), remap );
	}

	{
		int sum = 0;
		for(auto& e : m) sum += e();
		EXPECT_EQ(6, sum);
	}
}



TEST(Sparse_array, push_delete_compact_inplace) {

	Dynamic_Array<int>::SPARSE::CONSTRUCTED_FLAGS_INPLACE::COUNT m;
	push_delete_compact_common(m);
}





TEST(Sparse_array, push_delete_compact_bitset) {

	Dynamic_Array<int>::SPARSE::CONSTRUCTED_FLAGS_BITSET::COUNT m;
	push_delete_compact_common(m);
}






namespace {
	int g_destructors = 0;
	int g_constructors = 0;
}

TEST(Sparse_array, push_delete_compact_inplace_nontrivial) {

	struct S {
		int val;

		S(int v) : val(v) {
			++g_constructors;
		}
		
		operator int() const {return val;}

		~S() {
			++g_destructors;
		}

		S(S&& o) : val(o.val) {
			++g_constructors;
		}
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Dynamic_Array<S>::SPARSE::CONSTRUCTED_FLAGS_INPLACE::COUNT m;
		push_delete_compact_common(m);
	}

	EXPECT_EQ(g_constructors, g_destructors);
}






TEST(Sparse_array, copy_container_exists) {
	struct S {
		S()  { ++g_constructors; }
		S(const S&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Dynamic_Array<S>::SPARSE::CONSTRUCTED_FLAGS block;
		block.emplace_back();
		block.emplace_back();
		block.emplace_back();

		auto block2 = block;
		block2 = block;
	}

	EXPECT_EQ(g_constructors, g_destructors);
}


TEST(Sparse_array, move_container_exists) {
	struct S {
		S()  { ++g_constructors; }
		S(S&&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Dynamic_Array<S>::SPARSE::CONSTRUCTED_FLAGS::INPLACE_BUFFER<2> block(10);
		block.emplace_back();
		block.emplace_back();
		block.emplace_back();

		auto block2 = std::move(block);
		EXPECT_EQ(13, block2.domain());

		block = std::move(block2);
		EXPECT_EQ(13, block.domain());
	}

	EXPECT_EQ(g_constructors, g_destructors);
}









