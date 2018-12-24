#pragma once







#include <type_traits> // for `std::integral_constant`
#include <iostream>	   // for `std::cout`
#include <iomanip>	   // for `std::boolalpha`
#include <algorithm>   // for `std::sort`






namespace salgo {






//
// MEMBER DETECTOR
//
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
//
#define GENERATE_HAS_MEMBER(member) \
\
template < class T > \
class _Class_Has_Member__##member \
{ \
	struct Fallback { int member; }; \
	struct Derived : T, Fallback { }; \
\
	template<typename U, U> struct Check; \
\
	typedef char ArrayOfOne[1]; \
	typedef char ArrayOfTwo[2]; \
\
	template<typename U> static ArrayOfOne & func(Check<int Fallback::*, &U::member> *); \
	template<typename U> static ArrayOfTwo & func(...); \
  public: \
	enum { value = sizeof(func<Derived>(0)) == 2 }; \
}; \
\
template<class T> \
using _Has_Member__##member = std::conditional_t< std::is_class_v<T>, _Class_Has_Member__##member<T>, std::false_type>; \
\
template<class X> \
static constexpr bool has_member__##member = _Has_Member__##member<X>::value;








//
// ENUM CLASS BITMASKS
//
// http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks/
//

#define ENABLE_BITWISE_OPERATORS(X) \
\
constexpr X operator | (X lhs, X rhs)\
{ \
	using underlying = typename std::underlying_type_t<X>;\
	return static_cast<X> ( \
		static_cast<underlying>(lhs) |\
		static_cast<underlying>(rhs) \
	);\
} \
 \
constexpr X operator & (X lhs, X rhs) \
{ \
	using underlying = typename std::underlying_type_t<X>;\
	return static_cast<X> ( \
		static_cast<underlying>(lhs) &\
		static_cast<underlying>(rhs) \
	);\
} \
 \
constexpr X operator ! (X x) \
{ \
	using underlying = typename std::underlying_type_t<X>;\
	return static_cast<X> ( \
		! static_cast<underlying>(x) \
	);\
} \
 \
constexpr X operator ~ (X x) \
{ \
	using underlying = typename std::underlying_type_t<X>;\
	return static_cast<X> ( \
		~ static_cast<underlying>(x) \
	);\
}













#define FORWARDING_CONSTRUCTOR(SELF,BASE) \
	template<class... _ARGS> \
	SELF(_ARGS&&... args) : BASE( std::forward<_ARGS>(args)... )

#define FORWARDING_CONSTRUCTOR_1(SELF,BASE) \
	template<class A> \
	SELF(A&& a) : BASE( std::forward<A>(a) )

#define FORWARDING_CONSTRUCTOR_2(SELF,BASE) \
	template<class A, class B> \
	SELF(A&& a, B&& b) : BASE( std::forward<A>(a), std::forward<B>(b) )




#define FORWARDING_CONSTRUCTOR_BRACES(SELF,BASE) \
	template<class... ARGS> \
	SELF(ARGS&&... args) : BASE{ std::forward<ARGS>(args)... }

#define FORWARDING_CONSTRUCTOR_1_BRACES(SELF,BASE) \
	template<class A> \
	SELF(A&& a) : BASE{ std::forward<A>(a) }

#define FORWARDING_CONSTRUCTOR_2_BRACES(SELF,BASE) \
	template<class A, class B> \
	SELF(A&& a, B&& b) : BASE{ std::forward<A>(a), std::forward<B>(b) }





#define FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(SELF,BASE) \
  template<class _X> SELF(std::initializer_list<_X>&& il) : BASE( std::move(il) )



#define EXPLICIT_FORWARDING_CONSTRUCTOR(SELF,BASE) \
  template<class... ARGS> \
  explicit SELF(ARGS&&... args) : BASE( std::forward<ARGS>(args)... )

#define EXPLICIT_FORWARDING_CONSTRUCTOR_BRACES(SELF,BASE) \
  template<class... ARGS> \
  explicit SELF(ARGS&&... args) : BASE{ std::forward<ARGS>(args)... }

#define EXPLICIT_FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(SELF,BASE) \
  template<class _X> explicit SELF(std::initializer_list<_X>&& il) : BASE( std::move(il) )





#define FORWARDING_ASSIGNMENT(BASE) template<class X> auto& operator=(X&& x) { return BASE::operator=( std::forward<X>(x) ); }





//
// CRTP - common
//

#define CRTP_COMMON(THIS_BASE, CRTP_DERIVED)\
  private:\
	auto& _self()	   { return *static_cast<	   CRTP_DERIVED* >(this); }\
	auto& _self() const { return *static_cast< const CRTP_DERIVED* >(this); }

	// auto&& _rv_self()	   { return std::move( _self() ); }
	// auto&& _rv_self() const { return std::move( _self() ); }












//
// access first or last elements of containers
//
struct First_Tag {};
struct Last_Tag {};

#define FIRST salgo::First_Tag()
#define LAST salgo::Last_Tag()









#define ADD_MEMBER(member) \
template<class TYPE, bool> struct Add_##member { \
	TYPE member = TYPE(); \
	FORWARDING_CONSTRUCTOR_BRACES(Add_##member, member) {} \
}; \
template<class TYPE> struct Add_##member<TYPE,false> {};









#define ADD_MEMBER_STORAGE(member) \
template<class TYPE, bool> struct Add_Storage_##member { \
	::salgo::Stack_Storage<TYPE> member; \
}; \
template<class TYPE> struct Add_Storage_##member<TYPE,false> {};







// todo: replace std::sort maybe
template<class CONT>
void sort(CONT& cont) { std::sort(cont.begin(), cont.end()); }







// TODO: add template with enable_if is_integral
inline bool is_power_of_2(unsigned int x) {
	return (x&(x-1)) == 0;
}

inline bool is_power_of_2(unsigned long long x) {
	return (x&(x-1)) == 0;
}











} // namespace salgo

