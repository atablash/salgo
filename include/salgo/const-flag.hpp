#pragma once



#include <functional>



namespace salgo {



//
// const flag
//
enum class Const_Flag {
	MUTAB = 0,
	CONST = 1
};

namespace {
	constexpr auto MUTAB = Const_Flag::MUTAB;
	constexpr auto CONST = Const_Flag::CONST;
};

template<class T, Const_Flag c> using Const = std::conditional_t<c == CONST, const T, T>;




} // namespace salgo

