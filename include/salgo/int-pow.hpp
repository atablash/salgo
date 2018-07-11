#pragma once


namespace salgo {




template<class T, class E, class OP>
T int_pow(T a, E exp,
		const OP& op = [](T& a, const T& b){ a *= b; },
		const T& neutral = 1) {

	auto r = neutral;
	while(exp) {
		if(exp&1) {
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
	auto lambda = [](T& a, const T& b){ a *= b; };
	return int_pow<T,E,decltype(lambda)>(a, exp, lambda, 1);
}





} // namespace salgo

