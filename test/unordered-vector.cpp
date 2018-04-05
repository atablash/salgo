#include <salgo/unordered-vector.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;






TEST(Unordered_vector, test_push_delete) {

	Unordered_Vector<int> m;
	m.add(1); //
	m.add(2);
	m.add(3); //
	m.add(4);
	m.add(5); //

	EXPECT_EQ(5, m.size());

	EXPECT_EQ(1, m[0].val());
	EXPECT_EQ(2, m[1].val());
	EXPECT_EQ(3, m[2].val());
	EXPECT_EQ(4, m[3].val());
	EXPECT_EQ(5, m[4].val());

	{
		int sum = 0;
		for(const auto& e : m) sum += e.val();
		EXPECT_EQ(15, sum);
	}

	m[0].erase();
	m[2].erase();
	m[2].erase();

	EXPECT_EQ(2, m.size());

	{
		int sum = 0;
		for(const auto& e : m) sum += e.val();
		EXPECT_EQ(7, sum);
	}
}














