#pragma once

#include "int-pow.hpp"

#include <cstdint>
#include <iostream> // for overriding `ostream::operator<<`


namespace salgo {




template<int32_t MOD = (1u<<31)-1>
class Modulo {
public:
	using H = uint32_t;

private:
	H h = 0;

public:
	Modulo() = default;
	Modulo(uint32_t x) : h( x % MOD ) {}

private:
	static void _check(H h) {
		DCHECK_GE(h, 0);
		DCHECK_LT(h, MOD);
	}

public:
	Modulo& operator+=(const Modulo& o) {
		_check(h); _check(o.h);
		h += o.h;
		if(h >= MOD) h -= MOD;
		_check(h);
		return *this;
	}

	Modulo& operator-=(const Modulo& o) {
		_check(h); _check(o.h);
		h = MOD + h - o.h;
		if(h >= MOD) h -= MOD;
		_check(h);
		return *this;
	}

	Modulo& operator*=(const Modulo& o) {
		_check(h); _check(o.h);
		h = (1ULL * h * o.h) % MOD;
		_check(h);
		return *this;
	}

	Modulo& operator/=(const Modulo& o) {
		_check(h); _check(o.h);
		//ASS(millerRabin(MOD));
		DCHECK_EQ(1, int_pow(o, MOD-2) * o);
		*this *= int_pow(o, MOD-2);
		_check(h);
		return *this;
	}


	Modulo operator+(const Modulo& o) const { return Modulo(*this) += o; }
	Modulo operator-(const Modulo& o) const { return Modulo(*this) -= o; }
	Modulo operator*(const Modulo& o) const { return Modulo(*this) *= o; }
	Modulo operator/(const Modulo& o) const { return Modulo(*this) /= o; }



	//
	// bitwise
	Modulo& operator|=(const Modulo& o) { h |= o.h; return *this; }
	Modulo& operator&=(const Modulo& o) { h &= o.h; return *this; }
	Modulo& operator^=(const Modulo& o) { h ^= o.h; return *this; }

	Modulo operator|(const Modulo& o) const { return Modulo(*this) |= o; }
	Modulo operator&(const Modulo& o) const { return Modulo(*this) &= o; }
	Modulo operator^(const Modulo& o) const { return Modulo(*this) ^= o; }
	//



	//
	// bitshift
	Modulo& operator>>=(int x) { h >>= x; return *this; }
	Modulo& operator<<=(int x) { h <<= x; h %= MOD; return *this; }

	Modulo operator>>(int x) { return Modulo(*this) >>= x; }
	Modulo operator<<(int x) { return Modulo(*this) <<= x; }
	//


	// comparison
	bool operator==(const Modulo& o) const { return h == o.h; }
	bool operator!=(const Modulo& o) const { return h != o.h; }
	bool operator<=(const Modulo& o) const { return h <= o.h; }
	bool operator>=(const Modulo& o) const { return h >= o.h; }
	bool operator< (const Modulo& o) const { return h <  o.h; }
	bool operator> (const Modulo& o) const { return h >  o.h; }


	Modulo& operator--()    { if(h == 0) h = MOD; --h; return *this; }
	Modulo& operator++()    { ++h; if(h == MOD) h = 0; return *this; }
	Modulo  operator--(int) { auto m = *this; --(*this); return m; }
	Modulo  operator++(int) { auto m = *this; ++(*this); return m; }



	explicit operator int() const { DCHECK((int)h >= 0); return h; }
	explicit operator auto&() const { _check(h); return h; }
};


template<int32_t MOD> bool operator==(uint32_t a, const Modulo<MOD>& b) { return b == a; }
template<int32_t MOD> bool operator!=(uint32_t a, const Modulo<MOD>& b) { return b != a; }
template<int32_t MOD> bool operator<=(uint32_t a, const Modulo<MOD>& b) { return b >= a; }
template<int32_t MOD> bool operator>=(uint32_t a, const Modulo<MOD>& b) { return b <= a; }
template<int32_t MOD> bool operator< (uint32_t a, const Modulo<MOD>& b) { return b >  a; }
template<int32_t MOD> bool operator> (uint32_t a, const Modulo<MOD>& b) { return b <  a; }





template<int32_t MOD>
std::ostream& operator<<(std::ostream& s, const Modulo<MOD>& m) {
	auto i = typename Modulo<MOD>::H(m); s << i; return s;
}





} // namespace salgo



