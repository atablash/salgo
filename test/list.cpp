#include "common.hpp"

#include <salgo/list>
#include <salgo/salgo-from-std-allocator>

#include <gtest/gtest.h>

#include <list>
#include <chrono>

using namespace salgo;
using namespace std;
using namespace std::chrono;





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







TEST(List, count_nontrivial) {
	using T = Movable;
	T::reset();

	{
		List<T> ::COUNTABLE m;
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
	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
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






TEST(List, std_alloc) {
	using T = Movable;
	T::reset();

	{
		T asdf(123);
		using Alloc = Salgo_From_Std_Allocator< std::allocator<T> >;
		salgo::List<T> ::COUNTABLE ::ALLOCATOR<Alloc> li;

		for(int i=0; i<10; ++i) {
			li.emplace_back( rand() );
			li.emplace_front( rand() );
		}

		for(int i=0; i<10; ++i) {
			int ith = 0;
			for(auto& e : li) {

				if(ith%2) {
					e.erase();
					li.emplace_front( rand() );
				}

				++ith;
			}
		}
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}



