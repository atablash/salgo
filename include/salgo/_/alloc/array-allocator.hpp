#pragma once

/*

Allocates elements on a vector-like memory block.

Constructed objects are NOT persistent - they can be moved in memory by this allocator.

Grows memory block similar to std::vector, when congestion is 0.5 or more.

When constructing a new element, it looks for a hole in circular fashion.

Multithreaded code: keep in mind that old objects can be moved when new objects are allocated!

*/

#include "../accessors.hpp"
#include "../memory-block.hpp"




namespace salgo::alloc::_::array_allocator {


template<
	class VAL,
	int ALIGN
>
struct Params;

template<class P, Const_Flag C>
class Accessor;

template<class P>
class End_Iterator;

template<class P, Const_Flag C>
class Iterator;

template<class P>
struct Context;

template<class P>
class Array_Allocator;

template<class P>
class With_Builder;



} // namespace salgo::alloc::_::array_allocator



namespace salgo::alloc {


template< class VAL = int > // TODO: make it compile with `void`
using Array_Allocator = _::array_allocator::With_Builder< _::array_allocator::Params<
	VAL,
	0 // ALIGN
>>;


} // namespace salgo::alloc




