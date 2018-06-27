#include <salgo/list.hpp>
#include <salgo/salgo-from-std-allocator.hpp>

#include <gtest/gtest.h>

#include <list>
#include <chrono>

using namespace salgo;
using namespace std;
using namespace std::chrono;



DECLARE_int32(perf);


TEST(List, simple) {

	List<int> m;
	m.emplace_back(3);
	m.emplace_back(4);
	m.emplace_back(5);
	m.emplace_front(2);
	m.emplace_front(1);

	vector<int> vals;

	for(auto& e : m) vals.push_back( e );

	EXPECT_EQ(vector<int>({1,2,3,4,5}), vals);
}


TEST(List, count) {

	List<int> :: COUNTABLE m;
	m.emplace_back(3);
	m.emplace_back(4);
	m.emplace_back(5);
	m.emplace_front(2);
	m.emplace_front(1);

	EXPECT_EQ(5, m.count());

	vector<int> vals;
	for(auto& e : m) vals.push_back( e );

	EXPECT_EQ(vector<int>({1,2,3,4,5}), vals);
}





static int g_constructors = 0;
static int g_destructors = 0;

TEST(List, count_nontrivial) {

	struct S {
		int val;

		S(int i) : val(i) { ++g_constructors; }
		S(S&& o) { ++g_constructors; val = o.val; }
		~S() { ++g_destructors; }

		operator int() const { return val; }
	};

	{
		List<S> ::COUNTABLE m;
		m.emplace_back(3);
		m.emplace_back(4);
		m.emplace_back(5);
		m.emplace_front(2);
		m.emplace_front(1);

		EXPECT_EQ(5, m.count());

		vector<int> vals;
		for(auto& e : m) vals.push_back( e() );

		EXPECT_EQ(vector<int>({1,2,3,4,5}), vals);
	}

	// destructors called?
	EXPECT_EQ(g_constructors, g_destructors);
}





TEST(List, erase) {
	List<int> ::COUNTABLE m;
	m.emplace_back(123);
	m.begin().accessor().erase();

	EXPECT_EQ(0, m.count());
}





TEST(List, no_invalidation) {
	List<int> m;
	m.emplace_back(11);
	m.emplace_back(2);
	m.emplace_back(13);

	{
		int sum = 0;
		for(auto& e : m) {
			sum += e;
			if(e >= 10) e.erase();
		}
		EXPECT_EQ(26, sum);
	}

	{
		int sum = 0;
		for(auto& e : m) sum += e;
		EXPECT_EQ(2, sum);
	}
}





