#pragma once

#include "common.hpp"

#include <Eigen/Dense>

#include <glog/logging.h>

namespace salgo {

	template<class F, int DIM>
	constexpr auto get_aabb(const Eigen::Matrix<F,DIM,1>& vec) {
		return Eigen::AlignedBox<F,DIM>(vec, vec); // TODO: create dummpy Aligned_Box type for points to save some space
	}

	// template<class T>
	// constexpr auto get_aabb(T) noexcept {
	// 	DCHECK(false) << "- please define either `T::get_aabb` or salgo::get_aabb(T)";
	// }

    GENERATE_HAS_MEMBER(get_aabb)

	template<class T>
	struct Get_Aabb {
		auto operator()(const T& t) const {
            if constexpr(has_member__get_aabb<T>) return t.get_aabb();
            else return ::salgo::get_aabb(t);
		}
	};





} // namespace salgo


