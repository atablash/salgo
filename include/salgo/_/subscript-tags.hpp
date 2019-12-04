#pragma once



namespace salgo {


//
// access first or last elements of containers
//
//namespace _ {
	struct First_Tag {};
	struct Last_Tag {};
	struct Any_Tag {};
//}
static constexpr auto FIRST = First_Tag();
static constexpr auto LAST  = Last_Tag();
static constexpr auto ANY   = Any_Tag();




} // namespace salgo
