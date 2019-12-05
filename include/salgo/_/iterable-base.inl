#pragma once

#include "map.inl"

namespace salgo {

template<class Derived>
struct Iterable_Base {
	
	template<class FUN>
	auto map(FUN&& fun) {
		return Map( _self(), std::forward<FUN>(fun) );
	}

	SALGO_CRTP_COMMON(Derived)
};

} // namespace salgo
