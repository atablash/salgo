#pragma once

#include "map.inl"
#include "reverse.inl"

namespace salgo {

template<class Derived>
struct Iterable_Base {
	
	template<class FUN>
	auto map(FUN&& fun) {
		return Map( _self(), std::forward<FUN>(fun) );
	}

	template<class FUN>
	auto map(FUN&& fun) const {
		return Map( _self(), std::forward<FUN>(fun) );
	}


	auto reversed() {
		return Reverse( _self() );
	}

	auto reversed() const {
		return Reverse( _self() );
	}


	SALGO_CRTP_COMMON(Derived)
};

} // namespace salgo
