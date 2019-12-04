#pragma once

#include "../int-pow.hpp"
#include "../dynamic-array.hpp"

#include <cstdint>
#include <iostream> // for overriding `ostream::operator<<`


namespace salgo::discr {



//
// fix modulo of negative first argument
//
template<class A, class B>
auto mod(const A& a, const B& b, std::enable_if_t<std::is_unsigned_v<A>>* = 0) {
	return a%b;
}

template<class A, class B>
auto mod(const A& a, const B& b, std::enable_if_t<std::is_signed_v<A>>* = 0) {
	if(a < 0) {
		auto r = a%b;
		return r ? b+r : 0;
	}
	else return a%b;
}




//
// multiplying big numbers modulo, without integer overflow
//
#ifdef __SIZEOF_INT128__

inline constexpr uint64_t mult_and_mod(const uint64_t& a, const uint64_t& b, const uint64_t& m) {
	return (unsigned __int128)a * b % m;
}

#else

inline constexpr uint64_t mult_and_mod(uint64_t a, uint64_t b, uint64_t n) {
	uint64_t r = 0;
	if (a >= n) a %= n;   /* Careful attention from the caller */
	if (b >= n) b %= n;   /* should make these unnecessary.    */
	if ((a|b) < (1ULL << 32)) return (a*b) % n;
	if (a < b) { uint64_t t = a; a = b; b = t; }
	if (n <= (1ULL << 63)) {
		while (b > 0) {
			if (b & 1)  { r += a;  if (r >= n) r -= n; }
			b >>= 1;
			if (b)      { a += a;  if (a >= n) a -= n; }
		}
	} else {
		while (b > 0) {
			if (b & 1)  r = ((n-r) > a) ? r+a : r+a-n;    /* r = (r + a) % n */
			b >>= 1;
			if (b)      a = ((n-a) > a) ? a+a : a+a-n;    /* a = (a + a) % n */
		}
	}
	return r;
}

#endif














} // namespace salgo::discr



