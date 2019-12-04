#pragma once

#include <Eigen/Dense>

namespace salgo::geom {



// segment methods on anything segment-like
template<class SEG>
auto segment_squared_length(const SEG& seg) {
	return (seg.vert(1).pos() - seg.vert(0).pos()).squaredNorm();
}

template<class SEG>
auto segment_length(const SEG& seg) {
	return (seg.vert(1).pos() - seg.vert(0).pos()).norm();
}

template<class SEG>
auto segment_trace(const SEG& seg) {
	return seg.vert(1).pos() - seg.vert(0).pos();
}

template<class SEG>
auto segment_aabb(const SEG& seg) {
	using Vector = std::remove_cv_t<
		std::remove_reference_t< decltype( seg.vert(0).pos() ) >
	>;

	using Scalar = std::remove_reference_t< decltype( std::declval<Vector>()[0] ) >;
	static constexpr auto Dim = Vector::SizeAtCompileTime;

	return Eigen::AlignedBox<Scalar,Dim> {
		seg.vert(0).pos().array().min( seg.vert(1).pos().array() ),
		seg.vert(0).pos().array().max( seg.vert(1).pos().array() )
	};
}


// add methods to anything segment-like (CRTP-style)
template<class Derived>
struct Segment_Base {
	auto length() const {
		return segment_length( _self() );
	}

	auto squared_length() const {
		return segment_squared_length( _self() );
	}

	auto trace() const {
		return segment_trace( _self() );
	}

	auto aabb() const {
		return segment_aabb( _self() );
	}

	SALGO_CRTP_COMMON(Derived)
};



};
