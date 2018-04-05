#pragma once


#include <Eigen/Dense>


namespace salgo {





template<class SCALAR, int DIM>
struct Segment {

	using Scalar = SCALAR;
	static constexpr int Dim = DIM;

	template<class... ARGS>
	Segment(ARGS&&... args) : verts{{ std::forward<ARGS>(args)... }} {}

	std::array<Eigen::Matrix<Scalar, Dim, 1>, 2> verts;

	inline auto trace() const {
		return verts[1] - verts[0];
	}

// TODO: eigen memory alignment!!!!
};





} // namespace salgo

