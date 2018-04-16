#include <salgo/vector.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;



DECLARE_int32(perf);






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

	EXPECT_TRUE(  m(1).exists() );

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






TEST(Dense_vector, push_pop_resize) {

	Vector<int> m;
	push_pop_resize_common(m);
}






namespace {
	int g_destructors = 0;
	int g_constructors = 0;
}

TEST(Dense_vector, push_delete_compact_inplace_nontrivial) {

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
		Vector<S> m;
		push_pop_resize_common(m);
	}

	EXPECT_EQ(g_constructors, g_destructors);
}





