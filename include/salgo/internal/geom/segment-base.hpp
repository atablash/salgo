#pragma once

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

private:
	auto& _self() const { return *static_cast<const Derived*>(this); }
};



};
