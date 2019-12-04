#pragma once

#include "../const-flag.hpp"

namespace salgo::graph::_::graph {



enum class Erasable {
	NOT_ERASABLE = 0,
	ERASABLE_HOLES,
	ERASABLE_REORDER
};
namespace {
	constexpr auto NOT_ERASABLE     = Erasable:: NOT_ERASABLE;
	constexpr auto ERASABLE_HOLES   = Erasable:: ERASABLE_HOLES;
	constexpr auto ERASABLE_REORDER = Erasable:: ERASABLE_REORDER;
}





template<
	bool _DIRECTED,
	bool _BACKLINKS, // directed graphs can have 'ins' field
	class _VERT_DATA,
	class _EDGE_DATA,
	class _VERT_EDGE_DATA, // TODO: split into OUT_DATA and IN_DATA
	//class _OUT_DATA,
	//class _IN_DATA,
	Erasable _VERTS_ERASABLE,
	Erasable _EDGES_ERASABLE,
	bool _EDGES_GLOBAL
>
struct Params;



// global edge
template<class P>
struct Edge;

template<class P>
struct Vert_Edge;

template<class P>
struct Vert;




template<class P>
struct Verts_Context;

template<class P>
struct Edges_Context;

template<class P, int oi>
struct Vert_Edges_Context;





template<class P, Const_Flag C>
class A_Verts;

template<class P, Const_Flag C>
class A_Edges;

template<class P, Const_Flag C, int oi>
class A_Vert_Edges;




template<class P>
class Graph;

template<class P>
struct With_Builder;

} // namespace salgo::graph::_::graph








namespace salgo::graph {


using Graph = _::graph::With_Builder< _::graph::Params<
	false, // directed
	false, // backlinks
	void, // vert data
	void, // edge data
	void, // vert-edge data
	_::graph::NOT_ERASABLE, // verts erasable
	_::graph::NOT_ERASABLE, // edges erasable
	false // edges_global
>>;


} // namespace salgo::graph

