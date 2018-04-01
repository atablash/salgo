#include <salgo/sparse-vector.hpp>

#include <gtest/gtest.h>

#include <chrono>

using namespace salgo;
using namespace std::chrono;









TEST(Sparse_vector, push_back_and_read) {

	Sparse_Vector<int> m;
	m.emplace_back(0);
	m.emplace_back(1);
	m.emplace_back(2);
	m.emplace_back(3);
	m.emplace_back(4);

	EXPECT_EQ(5, m.count());
	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	EXPECT_EQ(0, m[0]);
	EXPECT_EQ(1, m[1]);
	EXPECT_EQ(2, m[2]);
	EXPECT_EQ(3, m[3]);
	EXPECT_EQ(4, m[4]);
}







TEST(Sparse_vector, test_delete) {

	Sparse_Vector<int> m;
	m.emplace_back(1); //
	m.emplace_back(2);
	m.emplace_back(3); //
	m.emplace_back(4);
	m.emplace_back(5); //

	EXPECT_EQ(5, m.count());

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	m.erase(0);
	m.erase(2);
	m.erase(4);

	EXPECT_TRUE( m.exists(1));
	EXPECT_FALSE(m.exists(2));

	int sum = 0;
	for(const auto& e : m) sum += e;
	EXPECT_EQ(6, sum);
}








