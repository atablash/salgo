#pragma once


#include <glog/logging.h>


namespace salgo {




template<class T, class E, class OP>
T int_pow(T a, E exp, const OP& op, const T& neutral) {
	DCHECK(exp >= 0);

	auto r = neutral;
	while(exp != 0) {
		if((exp & 1) != 0) {
			op(r, a);
			--exp;
		}
		else{
			exp >>= 1;
			op(a, a);
		}
	}
	return r;
}

template<class T, class E>
T int_pow(const T& a, const E& exp) {
	auto lambda = [](T& aa, const T& bb){ aa *= bb; };
	return int_pow<T,E,decltype(lambda)>(a, exp, lambda, 1);
}





} // namespace salgo

