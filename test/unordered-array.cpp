#include <salgo/unordered-array>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;






TEST(Unordered_Array, push_delete) {

	Unordered_Array<int> m;
	m.add(1); //
	m.add(2);
	m.add(3); //
	m.add(4);
	m.add(5); //

	EXPECT_EQ(5, m.size());

	EXPECT_EQ(1, m[0]);
	EXPECT_EQ(2, m[1]);
	EXPECT_EQ(3, m(2));
	EXPECT_EQ(4, m(3)());
	EXPECT_EQ(5, m[4]);

	{
		int sum = 0;
		for(const auto& e : m) sum += e;
		EXPECT_EQ(15, sum);
	}

	m(0).erase();
	m(2).erase();
	m(2).erase();

	EXPECT_EQ(2, m.size());

	{
		int sum = 0;
		for(const auto& e : m) sum += e;
		EXPECT_EQ(7, sum);
	}
}


TEST(Unordered_Array, no_invalidation) {

	Unordered_Array<int> m;
	m.add(100); //
	m.add(2);
	m.add(300); //
	m.add(4);
	m.add(500); //

	{
		int sum = 0;
		int iters = 0;
		for(auto& e : m) {
			sum += e;
			++iters;

			if(e >= 100) e.erase();
		}
		EXPECT_EQ(906, sum);
		EXPECT_EQ(5, iters);
	}

	{
		int sum = 0;
		for(const auto& e : m) sum += e;
		EXPECT_EQ(6, sum);
	}
}



TEST(Unordered_Array, reversed_iteration) {
	Unordered_Array<int> v = {1, 2, 3, 4, 5};
	set<int> s;
	for(auto& e : v.reversed()) {
		s.insert(e);
		if(e == 3) {
			v(e.handle()).erase();
		}
	}
	EXPECT_EQ(5, (int)s.size());
	EXPECT_EQ(4, v.size());
}



TEST(Unordered_Array, forward_iteration_good) {
	Unordered_Array<int> v = {1, 2, 3, 4, 5};
	set<int> s;
	for(auto& e : v) {
		s.insert(e);
		if(e == 3) {
			e.erase();
		}
	}
	EXPECT_EQ(5, (int)s.size());
	EXPECT_EQ(4, v.size());
}


TEST(Unordered_Array, forward_iteration_bad) {
	Unordered_Array<int> v = {1, 2, 3, 4, 5};
	set<int> s;
	for(auto& e : v) {
		s.insert(e);
		if(e == 3) {
			v(e.handle()).erase(); // we loose `5`
		}
	}
	EXPECT_EQ(4, (int)s.size());
	EXPECT_EQ(4, v.size());
}




