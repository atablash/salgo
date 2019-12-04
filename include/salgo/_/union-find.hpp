#pragma once

#include "const-flag.hpp"

namespace salgo::_::union_find {


template<class DATA, bool COUNTABLE, bool COUNTABLE_SETS>
struct Params;


template<class P>
struct Node;



template<class P, Const_Flag C>
class Accessor;

template<class P>
struct End_Iterator;

template<class P, Const_Flag C>
class Iterator;

template<class P>
struct Context;


template<class P>
class Union_Find;

template<class P>
class With_Builder;


} // namespace salgo::_::union_find






namespace salgo {

using Union_Find = _::union_find::With_Builder< _::union_find::Params<
	void, // DATA
	false, // COUNTABLE
	false // COUNTABLE_SETS
>>;

} // namespace salgo


