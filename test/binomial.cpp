#include <salgo/binomial.hpp>

#include <gtest/gtest.h>


using namespace std;
using namespace salgo;




TEST(Binomial, simple) {
	Binomial<int> b;

	EXPECT_EQ(1, b(0,0));
	EXPECT_EQ(10, b(10,9));
	EXPECT_EQ(15, b(6,4));
	EXPECT_EQ(0, b(3,4));
	EXPECT_EQ(1, b(0,0));
}



