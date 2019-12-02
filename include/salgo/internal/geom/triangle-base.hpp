#pragma once

namespace salgo::geom {






// add methods to anything segment-like (CRTP-style)
template<class Derived>
struct Triangle_Base {
	

private:
	auto& _self() { return *static_cast<Derived*>(this); }
};



};
