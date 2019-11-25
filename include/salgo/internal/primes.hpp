#pragma once

#include "modulo-utils.hpp"

#include <array>

namespace salgo {



template<class T>
constexpr T _mr_pow(const T& test_prime, const T& d, const T& p) {
	T x = 1;
	T exp = d;
	T tp = test_prime;
	while(exp) {
		if(exp&1) {
			x = mult_and_mod(x, tp, p);
			--exp;
		}
		else {
			tp = mult_and_mod(tp, tp, p);
			exp >>= 1;
		}
	}
	return x;
}



template<class T>
constexpr bool _mr_check(T x, int r, const T& p) {
	bool did_break = false;
	for(int j=0; j<r-1; ++j) {
		x = mult_and_mod(x, x, p); // mod p
		if(x == 1) {
			return false;
		}
		if(x == p-1) {
			did_break = true;
			break;
		}
	}

	if(!did_break) {
		return false;
	}

	return true;
}


// miller-rabin
template<class T, class ARR>
constexpr bool miller_rabin(const T& p, const ARR& test_primes) {
	if((p&1) == 0) return false;
	if(p < 2) return false;

	T d = p-1;
	int r = __builtin_ctz(d);
	d >>= r;

	for(auto& test_prime : test_primes) {
		using ARR_T = std::make_unsigned_t<std::remove_reference_t<decltype(test_prime)>>;
		if((ARR_T)test_prime >= p) continue;

		// using ::LOCAL, because can't use default ::CONTEXED in constexpr function
		//auto x = int_pow( Modulo ::LOCAL(test_prime, p), d );

		// inlining int_pow and modulo here, because gcc 7.3.0 has trouble compiling them as constexpr
		// x <- test_prime ^ d
		T x = _mr_pow<T>( test_prime, d, p );

		if(x==1 || x==p-1) continue;

		if(!_mr_check(std::move(x), r, p)) return false;
	}

	return true;
}



// miller rabin for p_max == 4,759,123,141 ( > 2^32)
constexpr bool is_prime(const unsigned int p) {
    return miller_rabin(p, std::array<unsigned int, 3>{{2, 7, 61}});
}

constexpr bool is_prime(const int p) { if(p < 0) return false; return is_prime((unsigned int)p); }

// if n < 318,665,857,834,031,151,167,461 (>2^64), it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, and 37.
constexpr bool is_prime(const unsigned long long p) {
	return miller_rabin(p, std::array<unsigned long long, 12>{{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}});
}

constexpr bool is_prime(const long long p) { if(p < 0) return false; return is_prime((unsigned long long)p); }


} // namespace salgo
