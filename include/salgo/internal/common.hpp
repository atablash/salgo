#pragma once







#include <type_traits> // std::integral_constant
#include <iostream>	   // std::cout
#include <iomanip>	   // std::boolalpha
#include <algorithm>   // std::sort
// #include <functional>  // std::is_constructible_v






namespace salgo {







//
// macro overloading
// https://stackoverflow.com/a/27051616/1123898
//
#define VARGS_(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N 
#define VARGS(...) VARGS_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)





//
// MEMBER DETECTOR
//
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
//
#define _GENERATE_HAS_MEMBER_3(member, definition_args, name) \
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

#define _GENERATE_HAS_MEMBER_1(member)   _GENERATE_HAS_MEMBER_3(member, , member)

#define GENERATE_HAS_MEMBER(...)   CONCAT(_GENERATE_HAS_MEMBER_, VARGS(__VA_ARGS__))(__VA_ARGS__)











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
	template<class... _ARGS, class = decltype(BASE(std::declval<_ARGS>()...))> \
	SELF(_ARGS&&... args) : BASE( std::forward<_ARGS>(args)... )

#define FORWARDING_CONSTRUCTOR_VAR(SELF,VAR) \
	template<class... _ARGS, class = decltype(decltype(VAR)(std::declval<_ARGS>()...))> \
	SELF(_ARGS&&... args) : VAR( std::forward<_ARGS>(args)... )

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
namespace internal {
	struct First_Tag {};
	struct Last_Tag {};
	struct Any_Tag {};
}

static constexpr auto FIRST = salgo::internal::First_Tag();
static constexpr auto LAST  = salgo::internal::Last_Tag();
static constexpr auto ANY   = salgo::internal::Any_Tag();









#define ADD_MEMBER(member) \
template<class TYPE, bool> struct Add_##member { \
	TYPE member = TYPE(); \
	FORWARDING_CONSTRUCTOR_VAR(Add_##member, member) {} \
}; \
template<class TYPE> struct Add_##member<TYPE,false> {};









#define ADD_MEMBER_STORAGE(member) \
template<class TYPE, bool> struct Add_Storage_##member { \
	::salgo::Inplace_Storage<TYPE> member; \
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




struct Invalid_Type {};




namespace internal {
	enum class Iteration_Callback_Result {
		BREAK    = 0x4c8b3311, // magic numbers in case user forgets to return CONTINUE
		CONTINUE = 0x69f012b4
	};
}
static constexpr auto BREAK    = internal::Iteration_Callback_Result::BREAK;
static constexpr auto CONTINUE = internal::Iteration_Callback_Result::CONTINUE;




// https://akrzemi1.wordpress.com/2016/07/07/the-cost-of-stdinitializer_list/

// 1. Implementing a variadic logical AND
template <bool...> struct bool_sequence {};

template <bool... Bs>
using bool_and = std::is_same<bool_sequence<Bs...>, bool_sequence<(Bs || true)...>>;

template< class T, class... Args>
using is_constructible = bool_and<std::is_constructible_v<T,Args>...>;

# define REQUIRES(...)    class = std::enable_if_t<(__VA_ARGS__)>








} // namespace salgo



//
// here I wanted to define tuple interfaces using T::Tuple_Size - but seems it's not possible?
//
// GENERATE_HAS_MEMBER(Tuple_Size)
// template<class T, REQUIRES(has_member__Tuple_Size<T>)>
// class std::tuple_size<T> : public std::integral_constant<std::size_t, T::Tuple_Size> {}