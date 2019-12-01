#pragma once

#include "common.hpp"

#include <Eigen/Dense>

#include <glog/logging.h>

namespace salgo {


template<class F, int DIM>
class Singular_Aligned_Box {
	using Vector = Eigen::Matrix<F,DIM,1>;
	Vector _v;

public:
	FORWARDING_CONSTRUCTOR_VAR(Singular_Aligned_Box, _v) {}

public:
	auto& min() const { return _v; }
	auto& max() const { return _v; }

	auto& center() const { return _v; }

	template<class AABB>
	bool intersects(const AABB& o) const {
		return
			(_v.array() <= o.max().array()).all() &&
			(o.min().array() <= _v.array()).all();
	}
};

template<class F, int DIM>
constexpr auto get_aabb(const Eigen::Matrix<F,DIM,1>& vec) {
	// return Eigen::AlignedBox<F,DIM>(vec, vec);
	return Singular_Aligned_Box<F,DIM>{vec};
}

// template<class T>
// constexpr auto get_aabb(T) noexcept {
// 	DCHECK(false) << "- please define either `T::get_aabb` or salgo::get_aabb(T)";
// }

GENERATE_HAS_MEMBER(get_aabb)

template<class T>
struct Get_Aabb {
	auto operator()(const T& t) const {
		if constexpr(has_member__get_aabb< std::remove_reference_t<T> >) return t.get_aabb();
		else return ::salgo::get_aabb(t);
	}
};




template<class F, int DIM>
auto grown_aabb(const Eigen::AlignedBox<F,DIM>& aabb, const F& grow_amount) {
	using Vec = Eigen::Matrix<F,DIM,1>;

	auto r = aabb;
	r.min() -= Vec{grow_amount, grow_amount, grow_amount};
	r.max() += Vec{grow_amount, grow_amount, grow_amount};
	return r;
}




} // namespace salgo


