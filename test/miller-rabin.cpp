#include <salgo/discr/primes>

#include <gtest/gtest.h>


using namespace std;
using namespace salgo::discr;



TEST(Mult_and_mod, long_long) {
	unsigned long long big = -15;
	EXPECT_EQ( mult_and_mod(big+1, big+1, big), 1u );
}



TEST(Miller_Rabin, signed_int) {
	EXPECT_TRUE(  is_prime(1000000009) );
	EXPECT_TRUE(  is_prime(1000000007) );
	EXPECT_TRUE(  is_prime(100000007) );

	EXPECT_TRUE(  is_prime((1u<<31) - 1) ); // mersenne prime

	EXPECT_FALSE( is_prime(100000009) );
}


TEST(Miller_Rabin, unsigned_int) {
	EXPECT_FALSE(  is_prime(-1u) );
	EXPECT_TRUE(  is_prime(4'294'967'291u) ); // largest prime

	int num = 0;
	for(unsigned int i = -100u; i != 0; ++i) {
		num += is_prime(i);
	}
	EXPECT_EQ(num,4);
}


TEST(Miller_Rabin, long_long) {
	EXPECT_TRUE(  is_prime((1LL << 61)-1) ); // mersenne prime

	EXPECT_TRUE( is_prime(18'446'744'073'709'551'557ULL) ); // largest prime

	int num = 0;
	for(unsigned long long i = -100ULL; i != 0; ++i) {
		num += is_prime(i);
	}
	EXPECT_EQ(num,3);
}


