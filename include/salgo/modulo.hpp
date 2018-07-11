#pragma once

#include "int-pow.hpp"

#include <cstdint>


namespace salgo {




template<int32_t MOD = (1u<<31)-1>
class Modulo {
private:
	uint32_t _h = 0;

public:
	Modulo() = default;
	Modulo(uint32_t x) : _h( x % MOD ) {}

public:
	Modulo& operator+=(const Modulo& o) {
		_h += o._h;
		if(_h >= MOD) _h -= MOD;
		return *this;
	}

	Modulo& operator-=(const Modulo& o) {
		_h = MOD + _h - o._h;
		if(_h >= MOD) _h -= MOD;
		return *this;
	}

	Modulo& operator*=(const Modulo& o) {
		_h = (1ULL * _h * o._h) % MOD;
		return *this;
	}

	Modulo& operator/=(const Modulo& o) {
		//ASS(millerRabin(MOD));
		return (*this) *= int_pow(o, MOD-2);
	}


	Modulo operator+(const Modulo& o) const { return Modulo(*this) += o; }
	Modulo operator-(const Modulo& o) const { return Modulo(*this) -= o; }
	Modulo operator*(const Modulo& o) const { return Modulo(*this) *= o; }
	Modulo operator/(const Modulo& o) const { return Modulo(*this) /= o; }

	operator int() const { return _h; }
};





} // namespace salgo



