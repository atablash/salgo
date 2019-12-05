#pragma once

#include "dynamic-array.hpp"

#include "const-flag.hpp"

namespace salgo::_::unordered_array {


template<class VAL, class VECTOR>
struct Params;

template<class P>
struct Handle;

template<class P>
struct Index;




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
class Unordered_Array;

template<class P>
class With_Builder;



} // namespace salgo::_::Unordered_Array




namespace salgo {

template<class T>
using Unordered_Array = typename _::unordered_array::With_Builder< _::unordered_array::Params<
	T,
	salgo::Dynamic_Array<T>
>>;

} // namespace salgo

