#pragma once

#include "primes.hpp"

namespace salgo {
namespace internal {
namespace modulo_hash {



constexpr uint32_t prev_big_prime(uint32_t x) {
    for(;;) {
        --x;
        if(is_prime(x)) return x;
    }
}



template<int NUM_HASHES>
class Modulo_Hash : Modulo_Hash<NUM_HASHES-1> {
    using BASE = Modulo_Hash<NUM_HASHES-1>;

public:
    Modulo_Hash() = default;

    template<class T>
    Modulo_Hash(const T& t) : BASE(t), m(t) {}

    template<class T>
    Modulo_Hash& operator=(const T& t) {
        BASE::operator=(t);
        m = t;
    }

protected:
    static constexpr auto mod = prev_big_prime( BASE::mod );
    Modulo ::MOD<mod> m;

private:
    Modulo_Hash(const BASE& new_base, const Modulo ::MOD<mod> new_m)
        : BASE(new_base), m(new_m) {}

public:
    bool operator==(const Modulo_Hash& o) const {
        return m == o.m  &&  BASE::operator==(o);
    }

    bool operator!=(const Modulo_Hash& o) const {
        return !(*this == o);
    }


public:
    Modulo_Hash& operator+=(const Modulo_Hash& o) {
        BASE::operator+=(o);
        m += o.m;
        return *this;
    }

    Modulo_Hash& operator-=(const Modulo_Hash& o) {
        BASE::operator-=(o);
        m -= o.m;
        return *this;
    }

    Modulo_Hash& operator*=(const Modulo_Hash& o) {
        BASE::operator*=(o);
        m *= o.m;
        return *this;
    }

    Modulo_Hash& operator/=(const Modulo_Hash& o) {
        BASE::operator/=(o);
        m /= o.m;
        return *this;
    }

public:
    Modulo_Hash operator+(const Modulo_Hash& o) {
        return Modulo_Hash(BASE::operator+(o), m + o.m);
    }

    Modulo_Hash operator-(const Modulo_Hash& o) {
        return Modulo_Hash(BASE::operator-(o), m - o.m);
    }

    Modulo_Hash operator*(const Modulo_Hash& o) {
        return Modulo_Hash(BASE::operator*(o), m * o.m);
    }

    Modulo_Hash operator/(const Modulo_Hash& o) {
        return Modulo_Hash(BASE::operator/(o), m / o.m);
    }
};





template<>
class Modulo_Hash<0> {
protected:
    static constexpr uint32_t mod = (1u<<31) - 1;

public:
    Modulo_Hash() = default;

    template<class T>
    Modulo_Hash(const T&) {}

public:
    bool operator==(const Modulo_Hash&) const { return true; }
    bool operator!=(const Modulo_Hash&) const { return false; }

public:
    Modulo_Hash& operator+=(const Modulo_Hash&) { return *this; }
    Modulo_Hash& operator-=(const Modulo_Hash&) { return *this; }
    Modulo_Hash& operator*=(const Modulo_Hash&) { return *this; }
    Modulo_Hash& operator/=(const Modulo_Hash&) { return *this; }

public:
    Modulo_Hash operator+(const Modulo_Hash&) { return Modulo_Hash(); }
    Modulo_Hash operator-(const Modulo_Hash&) { return Modulo_Hash(); }
    Modulo_Hash operator*(const Modulo_Hash&) { return Modulo_Hash(); }
    Modulo_Hash operator/(const Modulo_Hash&) { return Modulo_Hash(); }
};







} // namespace modulo_hash
} // namespace internal


template<int NUM_HASHES>
using Modulo_Hash = internal::modulo_hash::Modulo_Hash<NUM_HASHES>;


} // namespace salgo
