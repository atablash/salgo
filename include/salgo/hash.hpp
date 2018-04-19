#pragma once




namespace salgo {

	template<class T>
	struct Member_Hash {
		size_t operator()(const T& t) const {
			return t.hash();
		}
	};


	GENERATE_HAS_MEMBER(hash)

	template<class T> using Hash = std::conditional_t<
		has_member__hash<T>,
		Member_Hash<T>,
		::std::hash<T>
	>;

} // namespace salgo


