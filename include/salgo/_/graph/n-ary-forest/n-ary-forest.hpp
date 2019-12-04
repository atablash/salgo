#pragma once

#include "../../alloc/array-allocator.hpp"
#include "../../const-flag.hpp"

namespace salgo::graph::_::n_ary_forest {


template<
	int N_ARY,
	class DATA,
	bool CHILD_LINKS,
	bool PARENT_LINKS,
	//bool EVERSIBLE,
	//class AGGREG, class PROPAG,
	class SUPPLIED_ALLOCATOR,
	int ALIGN_OVERRIDE
>
struct Params;


template<class P, Const_Flag C>
class Reference;

template<class P, Const_Flag C>
class Accessor;

template<class P>
class End_Iterator;

template<class P, Const_Flag C>
class Iterator;

template<class P>
struct Context;

template<class P>
struct Node;

template<class P>
class N_Ary_Forest;

template<class P>
struct With_Builder;


} // namespace salgo::graph::_::n_ary_forest



namespace salgo::graph {


// N_ARY is number of children per node
// if N_ARY==0, number of children is dynamic
template<int N_ARY, class DATA = void>
using N_Ary_Forest = _::n_ary_forest::With_Builder< _::n_ary_forest::Params <
	N_ARY,
	DATA,
	false, // child_links
	false, // parent_links
	salgo::alloc::Array_Allocator<>,
	0 // align override
>>;


using Binary_Forest = N_Ary_Forest< 2 >;
using Rooted_Forest = N_Ary_Forest< 0 /* dynamic */>;


} // namespace salgo::graph



