#include <salgo/discr/binomial>
#include <salgo/discr/modulo>

#include <gtest/gtest.h>


using namespace std;
using namespace salgo::discr;




TEST(Binomial, simple) {
	Binomial<int> b;

	EXPECT_EQ(1, b(0,0));
	EXPECT_EQ(10, b(10,9));
	EXPECT_EQ(15, b(6,4));
	EXPECT_EQ(0, b(3,4));
	EXPECT_EQ(1, b(0,0));
}


TEST(Binomial, modulo) {
	Binomial<Modulo ::MOD<1'000'000'000 + 7>> b;

	EXPECT_EQ(538992043, b(100, 50));
	EXPECT_EQ(645441863, b(90, 80));
	EXPECT_EQ(15, b(6,4));
	EXPECT_EQ(1, b(0,0));
}

