#pragma once

#include "mesh.hpp"
#include "mesh-data.hpp"
#include "mesh-accessors.hpp"

// salgo
#include "../../../alloc/array-allocator.hpp"
#include "../../../dynamic-array.inl"
#include "../../../hash-table.hpp"

#include <Eigen/Dense>


namespace salgo::geom::g3d::_::mesh {




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
	class _SCALAR,
	class _VERT_DATA,
	class _SUBVERT_DATA,
	class _POLY_DATA,
	class _POLYVERT_DATA,
	class _POLYEDGE_DATA,
	Erasable _VERTS_ERASABLE,
	Erasable _POLYS_ERASABLE,
	bool _EDGE_LINKS,
	bool _VERT_POLY_LINKS
>
struct Params {
	using Scalar         = _SCALAR;
	using Vector         = Eigen::Matrix<Scalar,3,1>;

	using Vert_Data      = _VERT_DATA;
	using Subvert_Data   = _SUBVERT_DATA;
	using Poly_Data      = _POLY_DATA;
	using PolyVert_Data = _POLYVERT_DATA;
	using PolyEdge_Data = _POLYEDGE_DATA;



	static constexpr Erasable Verts_Erasable_Mode = _VERTS_ERASABLE;
	static constexpr auto Verts_Erasable = Verts_Erasable_Mode != NOT_ERASABLE;

	static constexpr Erasable Polys_Erasable_Mode = _POLYS_ERASABLE;
	static constexpr auto Polys_Erasable = Polys_Erasable_Mode != NOT_ERASABLE;

	static constexpr bool Has_Edge_Links      = _EDGE_LINKS;
	static constexpr bool Has_Vert_Poly_Links = _VERT_POLY_LINKS;



	static constexpr bool Has_Vert_Data =
		!std::is_same_v<Vert_Data, void>;

	static constexpr bool Has_Subvert_Data =
		!std::is_same_v<Subvert_Data, void>;

	static constexpr bool Has_Poly_Data =
		!std::is_same_v<Poly_Data, void>;

	static constexpr bool Has_PolyVert_Data =
		!std::is_same_v<PolyVert_Data, void>;

	static constexpr bool Has_PolyEdge_Data =
		!std::is_same_v<PolyEdge_Data, void>;



	using Vert = mesh::Vert<Params>;
	using Poly = mesh::Poly<Params>;

	using Verts = std::conditional_t<Verts_Erasable_Mode == ERASABLE_HOLES,
		alloc::Array_Allocator<Vert>,
		std::conditional_t< Verts_Erasable_Mode == ERASABLE_REORDER,
			salgo::Dynamic_Array< Vert >,
			salgo::Dynamic_Array< Vert > // NOT_ERASABLE
		>
	>;

	using Polys = std::conditional_t<Polys_Erasable_Mode == ERASABLE_HOLES,
		alloc::Array_Allocator<Poly>,
		std::conditional_t< Polys_Erasable_Mode == ERASABLE_REORDER,
			salgo::Dynamic_Array< Poly >,
			salgo::Dynamic_Array< Poly > // NOT_ERASABLE
		>
	>;

	using   H_Vert = typename Verts::Handle;
	using  SH_Vert = typename Verts::Handle_Small;
	using IDX_Vert = typename Verts::Index;

	using   H_Poly = typename Polys::Handle;
	using  SH_Poly = typename Polys::Handle_Small;
	using IDX_Poly = typename Polys::Index;


	// struct H_Subvert : Int_Handle_Base<H_Subvert> { FORWARDING_CONSTRUCTOR(H_Subvert, Int_Handle_Base<H_Subvert>) {} };
	// using SH_Subvert = H_Subvert;

	using  H_PolyVert = mesh::H_PolyVert<Params>;
	using SH_PolyVert = H_PolyVert;


	using  H_PolyEdge = mesh::H_PolyEdge<Params>;
	using SH_PolyEdge = H_PolyEdge;


	using Vert_Poly_Links = salgo::Hash_Table<H_PolyVert>;
	using H_VertPoly = mesh::H_VertPoly<Params>;

	using Mesh = mesh::Mesh<Params>;

	using Verts_Context = mesh::Verts_Context<Params>;
	using Polys_Context = mesh::Polys_Context<Params>;

	using PolyVerts_Context = mesh::PolyVerts_Context<Params>;
	using PolyEdges_Context = mesh::PolyEdges_Context<Params>;

	using VertPolys_Context = mesh::VertPolys_Context<Params>;

	//
	// accessors / iterators
	//
	// template<Const_Flag C> using A_Verts = mesh::A_Verts<Params,C>;
	// template<Const_Flag C> using A_Polys = mesh::A_Polys<Params,C>;

	// template<Const_Flag C> using A_VertPolys = mesh::A_VertPolys<Params,C>;
	// template<Const_Flag C> using A_PolyVerts = mesh::A_PolyVerts<Params,C>;
	// template<Const_Flag C> using A_PolyEdges = mesh::A_PolyEdges<Params,C>;



	template<Const_Flag C, class... ARGS>
	static auto create_verts_accessor(ARGS&&... args) { return mesh::A_Verts<Params,C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_polys_accessor(ARGS&&... args) { return mesh::A_Polys<Params,C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_polyEdges_accessor(ARGS&&... args) { return mesh::A_PolyEdges<Params,C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_polyVerts_accessor(ARGS&&... args) { return mesh::A_PolyVerts<Params,C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_vertPolys_accessor(ARGS&&... args) { return mesh::A_VertPolys<Params,C>( std::forward<ARGS>(args)... ); }


	// access raw mesh data
	static auto& raw(      Mesh& m, IDX_Vert h) {  return m._vs[h];  }
	static auto& raw(const Mesh& m, IDX_Vert h) {  return m._vs[h];  }

	static auto& raw(      Mesh& m, H_Poly h) {  return m._ps[h];  }
	static auto& raw(const Mesh& m, H_Poly h) {  return m._ps[h];  }

	static auto& raw(      Mesh& m, H_PolyVert h) {  return m._ps[ h.poly ].verts[ h.ith ];  }
	static auto& raw(const Mesh& m, H_PolyVert h) {  return m._ps[ h.poly ].verts[ h.ith ];  }

	static auto& raw(      Mesh& m, H_PolyEdge h) {  return m._ps[ h.poly ].edges[ h.ith ];  }
	static auto& raw(const Mesh& m, H_PolyEdge h) {  return m._ps[ h.poly ].edges[ h.ith ];  }

	static auto& raw_vs(      Mesh& m) { return m._vs; }
	static auto& raw_vs(const Mesh& m) { return m._vs; }

	static auto& raw_ps(      Mesh& m) { return m._ps; }
	static auto& raw_ps(const Mesh& m) { return m._ps; }
};



} // namespace salgo::geom::g3d::_::mesh
