#pragma once


namespace salgo {





// http://xoroshiro.di.unimi.it/
namespace {
namespace xoroshiro128x {
	uint64_t s[2] = {1,1};

	static inline uint64_t rotl(const uint64_t x, int k) {
		return (x << k) | (x >> (64 - k));
	}

	uint64_t next(void) {
		const uint64_t s0 = s[0];
		uint64_t s1 = s[1];
		const uint64_t result = s0 + s1;

		s1 ^= s0;
		s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
		s[1] = rotl(s1, 36); // c

		return result;
	}


	/* This is the jump function for the generator. It is equivalent
	   to 2^64 calls to next(); it can be used to generate 2^64
	   non-overlapping subsequences for parallel computations. */

	/*
	void jump(void) {
		static const uint64_t JUMP[] = { 0xbeac0467eba5facb, 0xd86b048b86aa9922 };

		uint64_t s0 = 0;
		uint64_t s1 = 0;
		for(int i = 0; i < int(sizeof JUMP / sizeof *JUMP); i++)
			for(int b = 0; b < 64; b++) {
				if (JUMP[i] & UINT64_C(1) << b) {
					s0 ^= s[0];
					s1 ^= s[1];
				}
				next();
			}

		s[0] = s0;
		s[1] = s1;
	}
	*/
}
}


inline uint32_t rand_32() {
	auto x = xoroshiro128x::next() & 0xffffffffu;
	//std::cout << x << std::endl;
	return x;
}

inline uint32_t rand_64() {
	auto x = xoroshiro128x::next();
	//std::cout << x << std::endl;
	return x;
}




} // namespace salgo
