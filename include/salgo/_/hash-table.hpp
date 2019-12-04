#pragma once

#include "alloc/array-allocator.hpp"
#include "hash.hpp"
#include "const-flag.hpp"

namespace salgo::_::hash_table {

template<class KEY, class VAL, class HASH, class ALLOCATOR, bool INPLACE>
struct Params;

template<class P>
struct Handle;


template<class P, Const_Flag C>
class Reference;

template<class P, Const_Flag C>
class Accessor;

template<class P>
struct End_Iterator;

template<class P, Const_Flag C>
class Iterator;


template<class P>
struct Context;


template<class P>
class Hash_Table;

template<class P>
class With_Builder;


} // namespace salgo::_::hash_table






namespace salgo {

template<
	class KEY,
	class VAL = void
>
using Hash_Table = typename _::hash_table::With_Builder< _::hash_table::Params<
	KEY,
	VAL,
	::salgo::Hash<KEY>, // HASH
	::salgo::alloc::Array_Allocator<int>, // ALLOCATOR (int will be rebound anyway), used only when not INPLACE
	std::is_move_constructible_v<KEY> && (std::is_same_v<VAL,void> || std::is_move_constructible_v<VAL>) // INPLACE
>>;


} // namespace salgo

