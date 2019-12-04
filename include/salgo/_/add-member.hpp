#pragma once

#include "constructor-macros.hpp"

#define SALGO_ADD_MEMBER(member) \
template<class TYPE, bool> struct Add_##member { \
	TYPE member = TYPE(); \
	FORWARDING_CONSTRUCTOR_VAR(Add_##member, member) {} \
	Add_##member(const Add_##member&) = default; \
	Add_##member(Add_##member&&) = default; \
	Add_##member& operator=(const Add_##member&) = default; \
	Add_##member& operator=(Add_##member&&) = default; \
}; \
template<class TYPE> struct Add_##member<TYPE,false> {};



#define SALGO_ADD_MEMBER_STORAGE(member) \
template<class TYPE, bool> struct Add_Storage_##member { \
	::salgo::Inplace_Storage<TYPE> member; \
}; \
template<class TYPE> struct Add_Storage_##member<TYPE,false> {};

