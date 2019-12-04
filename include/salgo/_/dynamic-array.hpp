#pragma once

#include "memory-block.hpp"

#include "const-flag.hpp"

namespace salgo::_::dynamic_array {


template<class P>
struct Handle;

template<class P>
struct Index;

template<class _VAL, bool _SPARSE, class _MEMORY_BLOCK>
struct Params;


template<class P, Const_Flag C>
class Accessor;

template<class P>
struct End_Iterator;

template<class P, Const_Flag C>
class Iterator;

template<class P>
struct Context;



template<class P>
class Dynamic_Array;

template<class P>
class With_Builder;


} // namespace salgo::_::Dynamic_Array






namespace salgo {


template< class T >
using Dynamic_Array = typename _::dynamic_array::With_Builder< _::dynamic_array::Params<
	T,
	false, // SPARSE
	Memory_Block<T>
>>;


template<class T>
using Sparse_Array = typename Dynamic_Array<T> ::SPARSE;



} // namespace salgo







// template<class X>
// struct std::hash<salgo::_::dynamic_array::Handle<X>> {
// 	size_t operator()(const salgo::_::dynamic_array::Handle<X>& h) const {
// 		return std::hash<salgo::_::dynamic_array::Handle_Int_Type>()(
// 			salgo::_::dynamic_array::Handle_Int_Type(h)
// 		);
// 	}
// };


