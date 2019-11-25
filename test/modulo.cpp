#include <salgo/modulo>

#include <gtest/gtest.h>

using namespace std;
using namespace salgo;




TEST(Modulo, simple) {
	Modulo ::MOD<11> m;
	m += 5; // 5
	m *= 200;
	m /= 100; // 10
	m -= 7; // 3
	EXPECT_EQ(3, m);
}



TEST(Modulo, contexted) {
	Modulo m;
	auto c = Modulo_Context(11);
	m += 5; // 5
	m *= 200;
	m /= 100; // 10
	m -= 7; // 3
	EXPECT_EQ(3, m);
}


template<uint32_t prime>
uint32_t big_prime() {
	Modulo ::MOD<prime> m;
	m += 811'111'111;
	m += 911'111'111;
	m -= 111'111'111;
	m *= 991'111'111;
	m /= 981'111'111;
	return static_cast<uint32_t>(m);
}


TEST(Modulo, big_prime_0) {
	const uint32_t prime = (1u<<31)-1;
	auto r = big_prime<prime>();
	EXPECT_EQ(r, 1'680'054'514u);
}

TEST(Modulo, big_prime_1) {
	const uint32_t prime = 2'147'483'629u;
	auto r = big_prime<prime>();
	EXPECT_EQ(r, 671'583'893u);
}

TEST(Modulo, big_prime_2) {
	const uint32_t prime = 2'147'483'587u;
	auto r = big_prime<prime>();
	EXPECT_EQ(r, 234'611'898u);
}



char palindrome(int size, int i) {
	if(i >= size/2) i = size-1 - i;
	return 'a' + (i % 26);
}

char almost_palindrome(int size, int i) {
	if(i >= size/2) i = size-1 - i;
	else if(i == size*1/3) ++i;
	return 'a' + (i % 26);
}


TEST(Modulo_Hash, palindrome) {
	using H = Modulo_Hash<5>;
	
	H h0, h1;
	H pow_26 = 1;

	const int N = 100'000;
	
	for(int i=0; i<N; ++i) {
		char c = palindrome(N, i);
		h0 = h0 * 26 + (c-'a');
		h1 += pow_26 * (c-'a');
		pow_26 *= 26;
	}

	EXPECT_EQ(h0, h1);
}


TEST(Modulo_Hash, almost_palindrome) {
	using H = Modulo_Hash<5>;
	
	H h0, h1;
	H pow_26 = 1;

	const int N = 100'000;
	
	for(int i=0; i<N; ++i) {
		char c = almost_palindrome(N, i);
		h0 = h0 * 26 + (c-'a');
		h1 += pow_26 * (c-'a');
		pow_26 *= 26;
	}

	EXPECT_NE(h0, h1);
}
