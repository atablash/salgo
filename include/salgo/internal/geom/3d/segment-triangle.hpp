#pragma once

namespace salgo::geom::geom_3d {




// ignores case when `point`'s projection is outside `triangle`
template<class Segment, class Triangle>
auto orient_segment_triangle(const Segment& segment, const Triangle& triangle) {
	using Vector = std::remove_cv_t<
		std::remove_reference_t< decltype(segment.vert(0).pos()) >
	>;

	using Scalar = std::remove_reference_t< decltype( std::declval<Vector>()[0] ) >;

    struct Result {
		bool cut = false;
		Vector point;
    };
    Result result;

	return result;
}


}
