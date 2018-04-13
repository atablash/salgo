#pragma once






//
// MEMBER DETECTOR
//
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
//

#include <type_traits> // To use 'std::integral_constant'.
#include <iostream>    // To use 'std::cout'.
#include <iomanip>     // To use 'std::boolalpha'.


#define GENERATE_HAS_MEMBER(member)                                                \
                                                                                   \
template < class T >                                                               \
class HasMember_##member                                                           \
{                                                                                  \
    struct Fallback { int member; };                                               \
    struct Derived : T, Fallback { };                                              \
                                                                                   \
    template<typename U, U> struct Check;                                          \
                                                                                   \
    typedef char ArrayOfOne[1];                                                    \
    typedef char ArrayOfTwo[2];                                                    \
                                                                                   \
    template<typename U> static ArrayOfOne & func(Check<int Fallback::*, &U::member> *); \
    template<typename U> static ArrayOfTwo & func(...);                            \
  public:                                                                          \
    typedef HasMember_##member type;                                               \
    enum { value = sizeof(func<Derived>(0)) == 2 };                                \
};                                                                                 \
                                                                                   \
template < class T >                                                               \
struct has_member_##member                                                         \
: public std::integral_constant<bool, HasMember_##member<T>::value>                \
{                                                                                  \
};                                                                                 \
                                                                                   \
template<>                                                                         \
struct has_member_##member<void>                                                   \
: public std::integral_constant<bool, false> {};








//
// ENUM CLASS BITMASKS
//
// http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks/
//



#define ENABLE_BITWISE_OPERATORS(X)                                               \
                                                                                  \
constexpr X operator | (X lhs, X rhs)                                             \
{                                                                                 \
    using underlying = typename std::underlying_type_t<X>;                        \
    return static_cast<X> (                                                       \
        static_cast<underlying>(lhs) |                                            \
        static_cast<underlying>(rhs)                                              \
    );                                                                            \
}                                                                                 \
                                                                                  \
constexpr X operator & (X lhs, X rhs)                                             \
{                                                                                 \
    using underlying = typename std::underlying_type_t<X>;                        \
    return static_cast<X> (                                                       \
        static_cast<underlying>(lhs) &                                            \
        static_cast<underlying>(rhs)                                              \
    );                                                                            \
}                                                                                 \
                                                                                  \
constexpr X operator ! (X x)                                                      \
{                                                                                 \
    using underlying = typename std::underlying_type_t<X>;                        \
    return static_cast<X> (                                                       \
        ! static_cast<underlying>(x)                                              \
    );                                                                            \
}                                                                                 \
                                                                                  \
constexpr X operator ~ (X x)                                                      \
{                                                                                 \
    using underlying = typename std::underlying_type_t<X>;                        \
    return static_cast<X> (                                                       \
        ~ static_cast<underlying>(x)                                              \
    );                                                                            \
}






#define FORWARDING_CONSTRUCTOR(SELF,BASE) \
  template<class... ARGS> \
  SELF(ARGS&&... args) : BASE( std::forward<ARGS>(args)... ) {}







//
// CRTP - common
//
// in DEBUG mode: make polymorphic for debugging with dynamic_cast
//
#ifndef NDEBUG
  #define CRTP_COMMON(THIS_BASE, CRTP_DERIVED)\
    private:\
      auto& _self()       { _check_crtp(); return *static_cast<       CRTP_DERIVED* >(this); }\
      auto& _self() const { _check_crtp(); return *static_cast< const CRTP_DERIVED* >(this); }\
    \
        public: virtual ~THIS_BASE() = default;\
        private: void _check_crtp() const {\
          DCHECK_EQ(\
            dynamic_cast<const CRTP_DERIVED*>(this),\
            static_cast<const CRTP_DERIVED*>(this)\
          );\
        }
#else
  #define CRTP_COMMON(THIS_BASE, CRTP_DERIVED)\
    private:\
      auto& _self()       { return *static_cast<       CRTP_DERIVED* >(this); }\
      auto& _self() const { return *static_cast< const CRTP_DERIVED* >(this); }
#endif

