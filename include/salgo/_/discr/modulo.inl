#pragma once

#include "modulo.hpp"

#include "primes.hpp"
#include "totient.hpp"

#include "../add-member.hpp"
#include "../dynamic-array.inl"
#include "../int-pow.hpp"


#include <cstdint>
#include <iostream> // for overriding `ostream::operator<<`


namespace salgo::discr::_::modulo {

using H = uint32_t; // hardcoded for now


struct Modulo_Dynamic_Context_Node {
	H mod;
	H totient;
};

static thread_local salgo::Dynamic_Array<Modulo_Dynamic_Context_Node> g_contexts;


struct Modulo_Dynamic_Context_RAII {

	Modulo_Dynamic_Context_RAII(H mod, H totient) {
		g_contexts.push_back({mod, totient});
	}

	Modulo_Dynamic_Context_RAII(H mod) {
		g_contexts.push_back({mod, mod-1});
	}

	~Modulo_Dynamic_Context_RAII() {
		g_contexts.pop_back();
	}
};



//#ifdef CONTEST_MODE
//	#define DEFAULT_MOD ((int)1e9 + 7)
//#else
//	#define DEFAULT_MOD ((1u<<31)-1)
//#endif



template<
	H MOD, // 0 if not static
	H TOTIENT,
	Type TYPE
>
struct Params {
	static constexpr H mod = MOD;
	static constexpr H totient = TOTIENT;
	static constexpr Type type = TYPE;

	static constexpr bool check() {
		if(type == Type::STATIC && mod == 0) return false;
		if(type != Type::STATIC && mod != 0) return false;
		return true;
	}
};



SALGO_ADD_MEMBER( mod );
SALGO_ADD_MEMBER( totient );



template<class P>
class Modulo :
		private Add_mod<H, P::type == Type::LOCAL || P::type == Type::LOCAL_WITH_TOTIENT>,
		private Add_totient<H, P::type == Type::LOCAL_WITH_TOTIENT> {

	using BASE_MOD = Add_mod<H, P::type == Type::LOCAL || P::type == Type::LOCAL_WITH_TOTIENT>;
	using BASE_TOTIENT =  Add_totient<H, P::type == Type::LOCAL_WITH_TOTIENT>;

	H h = 0;

private:
	H MOD() const {
		static_assert(P::check(), "invalid template arguments combination");

		if constexpr(P::type == Type::STATIC) return P::mod;
		else if constexpr(P::type == Type::CONTEXTED) {
			DCHECK(!g_contexts.empty()) << "you must create Modulo_Context first!";
			return g_contexts[LAST].mod;
		}
		else if constexpr(P::type == Type::LOCAL || P::type == Type::LOCAL_WITH_TOTIENT) return BASE_MOD::mod;
		//else static_assert(false, "invalid template parameters combination");
	}

	H TOTIENT() const {
		if constexpr(P::totient != 0) return P::totient; 
		else if constexpr(P::type == Type::CONTEXTED) {
			// both MOD and TOTIENT should have same source
			static_assert(P::mod == 0);
			DCHECK(!g_contexts.empty()) << "you must create Modulo_Context first!";
			return g_contexts[LAST].totient;
		}
		else if constexpr(P::type == Type::LOCAL) {
			DCHECK(is_prime(MOD())) << "MOD is not prime and TOTIENT requested, but not provided. use ::LOCAL_WITH_TOTIENT instead of ::LOCAL";
			return MOD() - 1; // assuming MOD is prime
		}
		else if constexpr(P::type == Type::LOCAL_WITH_TOTIENT) {
			return BASE_TOTIENT::totient;
		}
		//else static_assert(false, "invalid template parameters combination");
	}

private:
	// regular modulo, but try to avoid modulo if it's not too far from [0,MOD) range
	template<class T>
	H _fix_range(T x) const {
		if constexpr( P::mod > 0 ) static_assert(P::mod <= std::numeric_limits<T>::max());
		DCHECK_LE( MOD(), std::numeric_limits<T>::max() );
		T mod = MOD();
		if(x >= mod) {
			x -= mod;
			if(x < mod) return x;
			else return x % mod;
		}
		else if(x < 0) {
			x += mod;
			if(x >= 0) return x;
			else {
				H r = x % mod;
				return r ? mod+r : 0;
			}
		}
		else return x;
	}

public:
	Modulo() = default;

	template<class = std::enable_if_t<P::type != Type::LOCAL  &&  P::type != Type::LOCAL_WITH_TOTIENT>>
	Modulo(const H& x) : h( _fix_range(x) ) {}

	Modulo(const H& x, const H& m) : BASE_MOD(m), h( _fix_range(x)) { static_assert(P::type == Type::LOCAL); }
	Modulo(const H& x, const H& m, const H& t) : BASE_MOD(m), BASE_TOTIENT(t), h( _fix_range(x)) { static_assert(P::type == Type::LOCAL_WITH_TOTIENT); }

	// construct from `x`, but use MOD and TOTIENT same as `o` is using - useful for ::LOCAL and ::LOCAL_WITH_TOTIENT
	Modulo(const H& x, const Modulo& o) : BASE_MOD(o), BASE_TOTIENT(o), h( _fix_range(x) ) {}


	Modulo& operator=(const Modulo&) = default;

	template<class T, class = std::enable_if_t<std::numeric_limits<T>::is_integer>>
	Modulo& operator=(const T& o) {
		h = _fix_range(o);
		return *this;
	}


private:
	void _check(H hh) const {
		DCHECK_GE(hh, 0);
		DCHECK_LT(hh, MOD());
	}

public:
	Modulo& operator+=(const Modulo& o) {
		_check(h); _check(o.h);
		h += o.h;
		if(h >= MOD()) h -= MOD();
		_check(h);
		return *this;
	}

	Modulo& operator-=(const Modulo& o) {
		_check(h); _check(o.h);
		h = MOD() + h - o.h;
		if(h >= MOD()) h -= MOD();
		_check(h);
		return *this;
	}

	Modulo& operator*=(const Modulo& o) {
		_check(h); _check(o.h);
		if constexpr(false && P::mod < (1u<<31)  &&  P::mod > (1u<<30)) {
			// optimization for large MOD (IS ACTUALLY SLOWER ON MY MACHINE!)
			uint64_t r = 1ULL * h * o.h;
			const uint64_t HI = 0xff'ff'ff'ff'80'00'00'00ULL;
			const uint64_t LO = 0x00'00'00'00'7f'ff'ff'ffULL;
			for(int iter=0; iter<2; ++iter) {
				auto hi = r & HI;
				//if(!hi) break;
				r = (hi >> 31) * ((1u<<31) - MOD()) + (r & LO);
			}
			DCHECK_LT(r, 2*MOD());
			h = r;
			if(h >= MOD()) h -= MOD();
		}
		else {
			// TODO: maybe add branch if LARGE_MOD_OPTIMIZATION is enabled for dynamic MODs too
			h = (1ULL * h * o.h) % MOD();
		}
		_check(h);
		return *this;
	}

	Modulo& operator/=(const Modulo& o) {
		_check(h); _check(o.h);
		if constexpr(P::type == Type::STATIC) {
			if constexpr(is_prime(P::mod)) static_assert(P::totient == P::mod-1);
			else static_assert(P::totient < P::mod-1 && P::totient > 0, "you must provide a valid Euler's totient");
		}
		else {
			if(is_prime(MOD())) DCHECK_EQ(TOTIENT(), MOD()-1) << MOD() << " is prime, so totient should be " << MOD()-1;
			else DCHECK_LT(TOTIENT(), MOD()-1);
		}
		//ASS(millerRabin(MOD));
		//DCHECK_EQ(1, int_pow(o, MOD-2) * o);
		*this *= int_pow(o, TOTIENT()-1);
		_check(h);
		return *this;
	}


	Modulo operator+(const Modulo& o) const { return Modulo(*this) += o; }
	Modulo operator-(const Modulo& o) const { return Modulo(*this) -= o; }
	Modulo operator*(const Modulo& o) const { return Modulo(*this) *= o; }
	Modulo operator/(const Modulo& o) const { return Modulo(*this) /= o; }


	// comparison
	bool operator==(const Modulo& o) const { return h == o.h; }
	bool operator!=(const Modulo& o) const { return h != o.h; }
	bool operator<=(const Modulo& o) const { return h <= o.h; }
	bool operator>=(const Modulo& o) const { return h >= o.h; }
	bool operator< (const Modulo& o) const { return h <  o.h; }
	bool operator> (const Modulo& o) const { return h >  o.h; }


	Modulo& operator--()    { if(h == 0) h = MOD(); --h; return *this; }
	Modulo& operator++()    { ++h; if(h == MOD()) h = 0; return *this; }
	Modulo  operator--(int) { auto m = *this; --(*this); return m; }
	Modulo  operator++(int) { auto m = *this; ++(*this); return m; }



	explicit operator auto&() const { _check(h); return h; }
};


template<class P>
class With_Builder : public Modulo<P> {
	using BASE = Modulo<P>;

public:
	//using BASE::BASE;
	FORWARDING_CONSTRUCTOR(With_Builder, BASE) {}
	FORWARDING_ASSIGNMENT(BASE)
	//using BASE::operator=;

	template<H NEW_MOD>
	using MOD
		= With_Builder<Params<NEW_MOD, P::totient ? P::totient : totient(NEW_MOD), Type::STATIC>>;

	template<H NEW_TOTIENT>
	using TOTIENT
		= With_Builder<Params<P::mod, NEW_TOTIENT, Type::STATIC>>;

	using LOCAL
		= With_Builder<Params<0, 0, Type::LOCAL>>;

	using LOCAL_WITH_TOTIENT
		= With_Builder<Params<0, 0, Type::LOCAL_WITH_TOTIENT>>;
};




// comparison with integers... can it be done simpler? for ::LOCAL modulos it's not possible to construct from int
// todo: do comparison operators make any sense anyway?
template<class P>bool operator==(const H& h, const Modulo<P>& m) { return Modulo<P>(h,m) == m; }
template<class P>bool operator!=(const H& h, const Modulo<P>& m) { return Modulo<P>(h,m) != m; }
template<class P>bool operator<=(const H& h, const Modulo<P>& m) { return Modulo<P>(h,m) <= m; }
template<class P>bool operator>=(const H& h, const Modulo<P>& m) { return Modulo<P>(h,m) >= m; }
template<class P>bool operator< (const H& h, const Modulo<P>& m) { return Modulo<P>(h,m) <  m; }
template<class P>bool operator> (const H& h, const Modulo<P>& m) { return Modulo<P>(h,m) >  m; }

template<class P>bool operator==(const Modulo<P>& m, const H& h) { return m == Modulo<P>(h,m); }
template<class P>bool operator!=(const Modulo<P>& m, const H& h) { return m != Modulo<P>(h,m); }
template<class P>bool operator<=(const Modulo<P>& m, const H& h) { return m <= Modulo<P>(h,m); }
template<class P>bool operator>=(const Modulo<P>& m, const H& h) { return m >= Modulo<P>(h,m); }
template<class P>bool operator< (const Modulo<P>& m, const H& h) { return m <  Modulo<P>(h,m); }
template<class P>bool operator> (const Modulo<P>& m, const H& h) { return m >  Modulo<P>(h,m); }


template<class P>
std::ostream& operator<<(std::ostream& s, const Modulo<P>& m) {
	auto i = H(m);
	return s << i;
}


} // namespace salgo::discr::_::modulo

