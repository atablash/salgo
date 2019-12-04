#pragma once

#include "../linalg/aabb.hpp"

namespace salgo::geom {






// add methods to anything segment-like (CRTP-style)
template<class Derived>
struct Point_Base {
	auto aabb() const { return ::salgo::get_aabb( _self().pos() ); }

	SALGO_CRTP_COMMON(Derived)
};



};
