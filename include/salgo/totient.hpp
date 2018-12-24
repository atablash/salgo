#pragma once


#include "primes.hpp"



namespace salgo {

//
// euler's totient
//
template<class T>
inline constexpr auto totient(const T& t) {
	if(is_prime(t)) return t-1;
	else return 0u; // TODO: auto-compute?
}




} // namespace salgo


