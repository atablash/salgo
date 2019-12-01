#pragma once




namespace salgo {

namespace internal::hash {

	template<class T>
	struct Member_Hash {
		size_t operator()(const T& t) const {
			return t.hash();
		}
	};

	GENERATE_HAS_MEMBER(hash)

	template<class T>
	using Default_Hash = std::conditional_t<
		has_member__hash<T>,
		Member_Hash<T>,
		::std::hash<T>
	>;
}

template<class T>
struct Hash : internal::hash::Default_Hash<T> {};

template<class A, class B>
struct Hash<std::pair<A,B>> {
	size_t operator()(const std::pair<A,B>& p) const {
		auto cycle_bits = [](size_t x){
			auto amt = sizeof(x)/2 + 1;
			return (x << amt) ^ (x >> (sizeof(x) - amt));
		};
		return Hash<A>()(p.first) ^ cycle_bits( Hash<B>()(p.second) );
	}
};

} // namespace salgo


