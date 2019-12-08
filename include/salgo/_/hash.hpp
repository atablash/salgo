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
		constexpr std::size_t operator()(const std::pair<A,B>& p) const noexcept {
			constexpr auto cycle_bits = [](std::size_t x){
				auto amt = sizeof(x)/2 + 1;
				return (x << amt) ^ (x >> (sizeof(x) - amt));
			};
			return Hash<A>()(p.first) ^ cycle_bits( Hash<B>()(p.second) );
		}
	};

	template<class T, std::size_t SIZE>
	struct Hash<std::array<T,SIZE>> {
		constexpr std::size_t operator()(const std::array<T,SIZE>& arr) const noexcept {
			constexpr auto cycle_bits = [](std::size_t x) noexcept {
				auto amt = sizeof(x)/2 + 1;
				return (x << amt) ^ (x >> (sizeof(x) - amt));
			};

			if constexpr (SIZE == 0) return 0;

			std::size_t result = Hash<T>{}(arr[0]);
			for(std::size_t i=1; i<SIZE; ++i) {
				result = cycle_bits(result);
				result ^= Hash<T>{}(arr[i]);
			}

			return result;
		}
	};

} // namespace salgo


