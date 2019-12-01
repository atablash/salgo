#pragma once

#include "../../../const-flag.hpp"

namespace salgo::geom::geom_3d::internal::mesh {


template<class P> struct Verts_Context;
template<class P> struct VertPolys_Context;

template<class P> struct Polys_Context;
template<class P> struct PolyVerts_Context;
template<class P> struct PolyEdges_Context;



template<class P, Const_Flag C> class A_Verts;
template<class P, Const_Flag C> class A_Polys;
template<class P, Const_Flag C> class A_VertPolys;
template<class P, Const_Flag C> class A_PolyVerts;
template<class P, Const_Flag C> class A_PolyEdges;



} // namespace salgo::geom::geom_3d::internal::mesh
