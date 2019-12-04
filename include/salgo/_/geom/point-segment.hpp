#pragma once

namespace salgo::geom {


template<class POINT, class SEG>
auto orient_point_segment(const POINT& point, const SEG& segment) {
	using Vector = std::remove_const_t<
		std::remove_reference_t< decltype(point.pos()) >
	>;

	using Scalar = std::remove_reference_t< decltype( std::declval<Vector>()[0] ) >;

	Vector BA = segment.trace();
	Vector PA = point.pos() - segment.vert(0).pos();

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
	r.point_casted_onto_line = segment.vert(0).pos() + BA * ba_t;
	r.t = ba_t;

	return r;
}




};
