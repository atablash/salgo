#pragma once

#include <glog/logging.h>

#include <Eigen/Dense>


namespace salgo {





template<class SCALAR, int DIM>
class Segment {
	using Scalar = SCALAR;
	static constexpr int Dim = DIM;

	std::array<Eigen::Matrix<Scalar, Dim, 1>, 2> _verts;

public:
	template<class... ARGS>
	Segment(ARGS&&... args) : _verts{{ std::forward<ARGS>(args)... }} {}

	auto trace() const {
		return _verts[1] - _verts[0];
	}

	auto verts()       { return _verts; }
	auto verts() const { return _verts; }

	auto vert(int ith)       { DCHECK_LE(0, ith); DCHECK_LT(ith, 2); return _verts[ith]; }
	auto vert(int ith) const { DCHECK_LE(0, ith); DCHECK_LT(ith, 2); return _verts[ith]; }
};





} // namespace salgo

