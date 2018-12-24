#pragma once

#include "primes.hpp"

#include "totient.hpp"

#pragma once

#include "common.hpp"
#include "int-pow.hpp"
#include "vector.hpp"

#include <cstdint>
#include <iostream> // for overriding `ostream::operator<<`


namespace salgo {

namespace internal {
namespace modulo {

using H = uint32_t; // hardcoded for now


struct Modulo_Dynamic_Context_Node {
	H mod;
	H totient;
};

static thread_local salgo::Vector<Modulo_Dynamic_Context_Node> g_contexts;


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

enum class Type {
	STATIC,
	CONTEXTED,
	LOCAL,
	LOCAL_WITH_TOTIENT
};

// for use as template parameter
enum class Optional_Bool { UNSET=0, FALSE=1, TRUE=2 };
inline bool is_set  (Optional_Bool ob) { return ob != Optional_Bool::UNSET; }
inline bool is_true (Optional_Bool ob) { return ob != Optional_Bool::TRUE;  }
inline bool is_false(Optional_Bool ob) { return ob != Optional_Bool::FALSE; }


template<
	H MOD, // 0 if not static
	H TOTIENT,
	Type TYPE
>
struct Args {
	static constexpr H mod = MOD;
	static constexpr H totient = TOTIENT;
	static constexpr Type type = TYPE;

	static constexpr bool check() {
		if(type == Type::STATIC && mod == 0) return false;
		if(type != Type::STATIC && mod != 0) return false;
		return true;
	}
};



ADD_MEMBER( mod );
ADD_MEMBER( totient );



template<class ARGS>
class Modulo :
		private Add_mod<H, ARGS::type == Type::LOCAL || ARGS::type == Type::LOCAL_WITH_TOTIENT>,
		private Add_totient<H, ARGS::type == Type::LOCAL_WITH_TOTIENT> {

	using BASE_MOD = Add_mod<H, ARGS::type == Type::LOCAL || ARGS::type == Type::LOCAL_WITH_TOTIENT>;
	using BASE_TOTIENT =  Add_totient<H, ARGS::type == Type::LOCAL_WITH_TOTIENT>;

	H h = 0;

private:
	H MOD() const {
		static_assert(ARGS::check(), "invalid template arguments combination");

		if constexpr(ARGS::type == Type::STATIC) return ARGS::mod;
		else if constexpr(ARGS::type == Type::CONTEXTED) {
			DCHECK(!g_contexts.empty()) << "you must create Modulo_Context first!";
			return g_contexts[LAST].mod;
		}
		else if constexpr(ARGS::type == Type::LOCAL || ARGS::type == Type::LOCAL_WITH_TOTIENT) return BASE_MOD::mod;
		//else static_assert(false, "invalid template parameters combination");
	}

	H TOTIENT() const {
		if constexpr(ARGS::totient != 0) return ARGS::totient; 
		else if constexpr(ARGS::type == Type::CONTEXTED) {
			// both MOD and TOTIENT should have same source
			static_assert(ARGS::mod == 0);
			DCHECK(!g_contexts.empty()) << "you must create Modulo_Context first!";
			return g_contexts[LAST].mod;
		}
		else if constexpr(ARGS::type == Type::LOCAL) {
			DCHECK(is_prime(MOD())) << "MOD is not prime and TOTIENT requested, but not provided. use ::LOCAL_WITH_TOTIENT instead of ::LOCAL";
			return MOD() - 1; // assuming MOD is prime
		}
		else if constexpr(ARGS::type == Type::LOCAL_WITH_TOTIENT) {
			return BASE_TOTIENT::totient;
		}
		//else static_assert(false, "invalid template parameters combination");
	}

public:
	Modulo() = default;

	template<class = std::enable_if_t<ARGS::type != Type::LOCAL  &&  ARGS::type != Type::LOCAL_WITH_TOTIENT>>
	Modulo(const H& x) : h( x % MOD() ) {}

	Modulo(const H& x, const H& m) : BASE_MOD(m), h( x % MOD()) { static_assert(ARGS::type == Type::LOCAL); }
	Modulo(const H& x, const H& m, const H& t) : BASE_MOD(m), BASE_TOTIENT(t), h( x % MOD()) { static_assert(ARGS::type == Type::LOCAL_WITH_TOTIENT); }

	// construct from `x`, but use MOD and TOTIENT same as `o` is using - useful for ::LOCAL and ::LOCAL_WITH_TOTIENT
	Modulo(const H& x, const Modulo& o) : BASE_MOD(o), BASE_TOTIENT(o), h( x % MOD() ) {}


	Modulo& operator=(const Modulo&) = default;

	template<class T, class = std::enable_if_t<std::numeric_limits<T>::is_integer>>
	Modulo& operator=(const T& o) {
		DCHECK_GE(o, 0) << "as an optimization, no negative numbers allowed. it would require a branch to make % work correctly";
		h = o % MOD();
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
		h = (1ULL * h * o.h) % MOD();
		_check(h);
		return *this;
	}

	Modulo& operator/=(const Modulo& o) {
		_check(h); _check(o.h);
		if constexpr(ARGS::type == Type::STATIC) {
			if constexpr(is_prime(ARGS::mod)) static_assert(ARGS::totient == ARGS::mod-1);
			else static_assert(ARGS::totient < ARGS::mod-1, "you must provide a valid Euler's totient");
		}
		else {
			if(is_prime(MOD())) DCHECK_EQ(TOTIENT(), MOD()-1);
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


template<class ARGS>
struct With_Builder : Modulo<ARGS> {
	FORWARDING_CONSTRUCTOR(With_Builder, Modulo<ARGS>) {}
	FORWARDING_ASSIGNMENT(Modulo<ARGS>)

	template<H NEW_MOD>
	using MOD = With_Builder<Args<NEW_MOD, ARGS::totient ? ARGS::totient : totient(NEW_MOD), Type::STATIC>>;

	template<H NEW_TOTIENT>
	using TOTIENT = With_Builder<Args<ARGS::mod, NEW_TOTIENT, Type::STATIC>>;

	using LOCAL = With_Builder<Args<0, 0, Type::LOCAL>>;
	using LOCAL_WITH_TOTIENT = With_Builder<Args<0, 0, Type::LOCAL_WITH_TOTIENT>>;
};




// comparison with integers... can it be done simpler? for ::LOCAL modulos it's not possible to construct from int
template<class ARGS>bool operator==(const H& h, const Modulo<ARGS>& m) { return Modulo<ARGS>(h,m) == m; }
template<class ARGS>bool operator!=(const H& h, const Modulo<ARGS>& m) { return Modulo<ARGS>(h,m) != m; }
template<class ARGS>bool operator<=(const H& h, const Modulo<ARGS>& m) { return Modulo<ARGS>(h,m) <= m; }
template<class ARGS>bool operator>=(const H& h, const Modulo<ARGS>& m) { return Modulo<ARGS>(h,m) >= m; }
template<class ARGS>bool operator< (const H& h, const Modulo<ARGS>& m) { return Modulo<ARGS>(h,m) <  m; }
template<class ARGS>bool operator> (const H& h, const Modulo<ARGS>& m) { return Modulo<ARGS>(h,m) >  m; }

template<class ARGS>bool operator==(const Modulo<ARGS>& m, const H& h) { return m == Modulo<ARGS>(h,m); }
template<class ARGS>bool operator!=(const Modulo<ARGS>& m, const H& h) { return m != Modulo<ARGS>(h,m); }
template<class ARGS>bool operator<=(const Modulo<ARGS>& m, const H& h) { return m <= Modulo<ARGS>(h,m); }
template<class ARGS>bool operator>=(const Modulo<ARGS>& m, const H& h) { return m >= Modulo<ARGS>(h,m); }
template<class ARGS>bool operator< (const Modulo<ARGS>& m, const H& h) { return m <  Modulo<ARGS>(h,m); }
template<class ARGS>bool operator> (const Modulo<ARGS>& m, const H& h) { return m >  Modulo<ARGS>(h,m); }


template<class ARGS>
std::ostream& operator<<(std::ostream& s, const Modulo<ARGS>& m) {
	auto i = H(m);
	return s << i;
}




} // namespace modulo
} // namespace internal







using Modulo = typename internal::modulo::With_Builder< internal::modulo::Args<
	0,
	0,
	internal::modulo::Type::CONTEXTED
>>;




using Modulo_Context = internal::modulo::Modulo_Dynamic_Context_RAII;








} // namespace salgo



