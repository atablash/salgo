#pragma once

#include <iostream>

namespace salgo {



namespace _ {
namespace ratio {




template<
    class _T,
    bool _GCD
>
struct Context {

    using T = _T;
    static constexpr bool Gcd = _GCD;




    class Ratio {
        T numer = T(0);
        T denom = T(1);

    public:
        Ratio() = default;
        Ratio(const T& x) : numer(x) {}
        Ratio(const T& n, const T& d) : numer(n), denom(d) {}

        R& gcd() {
            if constexpr( _GCD ) {
                auto g = gcd(numer, denom);
                numer /= g;
                denom /= g;
                if(denom < 0) {
                    numer *= -1;
                    denom *= -1;
                }
                DCHECK_GT(denom, 0);
            }
            return *this;
        }

        const R& operator+() const { return *this; }

        R operator-() const {
            return R{ -numer, denom };
        }

        T eval() const {
            return numer / denom;
        }

        R& operator+=(const R& o) {
            return *this = *this + o;
        }

        R& operator-=(const R& o) {
            return *this = *this - o;
        }

        R& operator*=(const R& o) {
            return *this = *this * o;
        }

        R& operator/=(const R& o) {
            return *this = *this / o;
        }
    };


    template<class T>
    Ratio<T> operator*(const Ratio<T>& a, const Ratio<T>& b) {
        R<T> r = {a.numer * b.numer, a.denom * b.denom};
        return r.gcd();
    }

    template<class T>
    Ratio<T> operator/(const Ratio<T>& a, const Ratio<T>& b) {
        R<T> r = {a.numer * b.denom, a.denom * b.numer};
        return r.gcd();
    }


    template<class T>
    Ratio<T> operator+(const Ratio<T>& a, const Ratio<T>& b) {
        R<T> r = {
            a.numer * b.denom + b.numer * a.denom,
            a.denom * b.denom
        };
        return r.gcd();
    }

    template<class T>
    Ratio<T> operator-(const Ratio<T>& a, const Ratio<T>& b) {
        R<T> r = {
            a.numer * b.denom - b.numer * a.denom,
            a.denom * b.denom
        };
        return r.gcd();
    }


    template<class T>
    std::ostream& operator<<(std::ostream& os, const Ratio<T>& r) {
        return os << r.numer << "/" << r.denom;
    }





	struct With_Builder : Ratio {
        using BASE = Ratio;
        using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(With_Builder, Ratio) {}

		template<class NEW_ALLOCATOR>
		using GCD = typename Context<T, true> ::With_Builder;
	};

}; // struct Context

} // namespace ratio
} // namespace _







template<class T>
using Ratio = typename _::ratio::Context<
    T,
    false // GCD
> ::With_Builder;




} // namespace salgo

