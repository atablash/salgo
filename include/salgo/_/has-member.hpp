#pragma once

#include "macro-overloading.hpp"

#include <type_traits> // std::conditional_t

//
// MEMBER DETECTOR
//
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
//
#define _SALGO_GENERATE_HAS_MEMBER_3(member, definition_args, name) \
\
template < class T > \
class _Class_Has_Member__##name \
{ \
	struct Fallback { int member definition_args; }; \
	struct Derived : T, Fallback { }; \
\
	template<class U, U> struct Check; \
\
	typedef char ArrayOfOne[1]; \
	typedef char ArrayOfTwo[2]; \
\
	template<class U> static ArrayOfOne & test(Check<int (Fallback::*) definition_args, &U::member> *); \
	template<class U> static ArrayOfTwo & test(...); \
  public: \
	enum { value = sizeof(test<Derived>(0)) == 2 }; \
}; \
\
template<class T> \
using _Has_Member__##name = std::conditional_t< std::is_class_v<T>, _Class_Has_Member__##name<T>, std::false_type>; \
\
template<class X> \
static constexpr bool has_member__##name = _Has_Member__##name<X>::value;

#define _SALGO_GENERATE_HAS_MEMBER_1(member)   _SALGO_GENERATE_HAS_MEMBER_3(member, , member)

#define SALGO_GENERATE_HAS_MEMBER(...)   SALGO_CONCAT(_SALGO_GENERATE_HAS_MEMBER_, SALGO_VARGS(__VA_ARGS__))(__VA_ARGS__)


