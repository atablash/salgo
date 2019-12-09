#include <salgo/dynamic-array>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;





TEST(Dynamic_Array, copy) {
	Dynamic_Array<int> v = {1, 2, 3, 4, 5};
	auto other = v;
}




TEST(Dynamic_Array, accessor_operators) {
	Dynamic_Array<int> v = {1, 2, 3, 4, 5};
	for(auto& e : v) {
		++e;
		e += 10;
	}
	EXPECT_EQ(1+10+1, v[0]);
	EXPECT_EQ(1+10+5, v[4]);
}


TEST(Dynamic_Array, iterator_operators) {
	Dynamic_Array<int> v = {1, 1, 2, 100, 100, 100, 1, 1};
	int sum = 0;
	for(auto& e : v) {
		sum += e;
		if(e == 2) {
			// skip 1+2 elements after '2'
			e.iterator()++;
			e.iterator() += 2;
		}
	}
	EXPECT_EQ(1+1+2+1+1, sum);
}



TEST(Dynamic_Array, push_while_iterating) {
	Dynamic_Array<int> v = {1,2,3};
	int sum = 0;
	for(auto& e : v) {
		sum += e;
		if(e < 10) v.emplace_back(e+10);
	}
	// 1,2,3,11,12,13
	EXPECT_EQ(42, sum);
}








template<class MB>
static void push_pop_resize_common(MB& m) {
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

	m.pop_back();

	EXPECT_TRUE(  m(1).is_constructed() );

	EXPECT_EQ(4, m.count());

	{
		int sum = 0;
		for(const auto& e : m) sum += e();
		EXPECT_EQ(10, sum);
	}

	m.resize( m.size() + 2, 2 );

	{
		int sum = 0;
		for(const auto& e : m) sum += e();
		EXPECT_EQ(14, sum);
	}
}






TEST(Dynamic_Array, push_pop_resize) {
	Dynamic_Array<int> m;
	push_pop_resize_common(m);
}









namespace {
	int g_constructed = 0;
}

TEST(Dynamic_Array, constructors) {
	struct S {
		S()    { ++g_constructed; }
		S(S&&) { ++g_constructed; }
		~S()   { --g_constructed; }
	};

	g_constructed = 0;
	{
		Dynamic_Array<S> v(10);
		EXPECT_EQ(10, v.count());
		EXPECT_EQ(10, g_constructed);

		v.add();
		EXPECT_EQ(11, v.count());
		EXPECT_EQ(11, g_constructed);
	}

	EXPECT_EQ(0, g_constructed);
}










namespace {
	int g_destructors = 0;
	int g_constructors = 0;
}

TEST(Dynamic_Array, push_delete_compact_inplace_nontrivial) {

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
		Dynamic_Array<S> m;
		push_pop_resize_common(m);
	}

	EXPECT_EQ(g_constructors, g_destructors);
}



TEST(Dynamic_Array, assignment_operator) {
	Dynamic_Array<int> vec(1);

	vec(0) = 123;
	EXPECT_EQ(vec[0], 123);

	vec[0] = 234;
	EXPECT_EQ(vec(0), 234);
}



TEST(Dynamic_Array, access_last) {
	const Dynamic_Array<int> vec = {11, 22};
	EXPECT_EQ(vec[LAST], 22);
}




TEST(Accessor, print_to_ostream) {
	Dynamic_Array<int> v(1);

	std::ostringstream os, os_want;
	os << v(0);

	os_want << "0 @" << &v;

	EXPECT_EQ(os.str(), os_want.str());
}


