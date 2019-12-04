#pragma once

#include "has-member.hpp"

#include <cstddef> // std::size_t
#include <functional> // std::hash


namespace salgo::_::hash {

	template<class T>
	struct Member_Hash {
		std::size_t operator()(const T& t) const {
			return t.hash();
		}
	};

	SALGO_GENERATE_HAS_MEMBER(hash)

	template<class T>
	using Default_Hash = std::conditional_t<
		has_member__hash<T>,
		Member_Hash<T>,
		::std::hash<T>
	>;

} // namespace salgo::_::hash







namespace salgo {

	template<class T>
	struct Hash : _::hash::Default_Hash<T> {};

	template<class A, class B>
	struct Hash<std::pair<A,B>> {
		std::size_t operator()(const std::pair<A,B>& p) const {
			auto cycle_bits = [](std::size_t x){
				auto amt = sizeof(x)/2 + 1;
				return (x << amt) ^ (x >> (sizeof(x) - amt));
			};
			return Hash<A>()(p.first) ^ cycle_bits( Hash<B>()(p.second) );
		}
	};

} // namespace salgo


