#pragma once

#include "segment.hpp"

#include <Eigen/Dense>

namespace salgo {

namespace internal {
namespace orient_point_segment {

template<class Vector, class SEG>
auto impl(const Vector& point, const SEG& segment) {
	using Scalar = std::remove_reference_t< decltype( std::declval<Vector>()[0] ) >;

	Vector BA = segment.vert(1) - segment.vert(0);
	Vector PA = point - segment.vert(0);

	Scalar ba_t = BA.dot(PA) / BA.squaredNorm(); // [0..1] means casted point is on segment

	struct Result {
		Scalar dist_from_line_sqr;
		Scalar dist_from_line;
		Vector point_casted_onto_line;
		Scalar t;
	};

	Result r;
	r.dist_from_line_sqr = (PA - BA * ba_t).squaredNorm();
	r.dist_from_line = sqrt( r.dist_from_line_sqr );
	r.point_casted_onto_line = segment.vert(0) + BA * ba_t;
	r.t = ba_t;

	return r;
}

}
}



GENERATE_HAS_MEMBER(eval)


template<class VEC, class SEG>
auto orient_point_segment(const VEC& point, const SEG& segment) {
	// eval point if needed
	if constexpr(has_member__eval<VEC>) return internal::orient_point_segment::impl(point.eval(), segment);
	else return internal::orient_point_segment::impl(point, segment);
}




};
