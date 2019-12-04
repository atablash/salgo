#pragma once

#include "const-flag.hpp"

namespace salgo::_::memory_block {


template<class X>
struct Handle;


// same as Handle, but allow creation from `int`
template<class X>
struct Index;




template<class VAL, class ALLOCATOR, int STACK_BUFFER, bool DENSE,
		bool CONSTRUCTED_FLAGS_INPLACE, bool CONSTRUCTED_FLAGS_BITSET, bool COUNT, int ALIGN>
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
class Memory_Block;


template<class P>
class With_Builder;


} // namespace salgo::_::Memory_Block





namespace salgo {

template<
		class T
>
using Memory_Block = typename _::memory_block::With_Builder< _::memory_block::Params<
		T,
		std::allocator<T>, // ALLCOATOR
		0, // STACK_BUFFER
		false, // DENSE
		false, // CONSTRUCTED_FLAGS_INPLACE
		false, // CONSTRUCTED_FLAGS_BITSET
		false, // COUNT
		0 // ALIGN
>>;


} // namespace salgo

