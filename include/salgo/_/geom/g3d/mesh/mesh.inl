#pragma once

// mesh
#include "mesh-base.inl"
#include "mesh-params.inl"
#include "mesh-data.inl"
#include "mesh-accessors.inl"

// geom/3d
#include "../named-arguments.hpp"
#include "../mesh-utils.hpp"

#include "../../../template-macros.hpp"

#include <array>



namespace salgo::geom::g3d::_::mesh {

















template<class P>
class Mesh : protected P, public Mesh_Base<Mesh<P>> {
public:
	using Scalar          = typename P::Scalar;
	using Vector          = typename P::Vector;

	using Vert_Data      = typename P::Vert_Data;
	using Subvert_Data   = typename P::Subvert_Data;
	using Poly_Data      = typename P::Poly_Data;
	using PolyVert_Data = typename P::PolyVert_Data;
	using PolyEdge_Data = typename P::PolyEdge_Data;

	static constexpr auto Verts_Erasable_Mode = P::Verts_Erasable_Mode;
	static constexpr auto Verts_Erasable = P::Verts_Erasable;
	
	static constexpr auto Polys_Erasable_Mode = P::Polys_Erasable_Mode;
	static constexpr auto Polys_Erasable = P::Polys_Erasable;


	// template<Const_Flag C>
	// using  A_Vert      = typename P::template  A_Vert<C>;
	using  H_Vert      = typename P::          H_Vert;
	using SH_Vert      = typename P::         SH_Vert;
	
	// template<Const_Flag C>
	// using  A_Poly      = typename P::template  A_Poly<C>;
	using  H_Poly      = typename P::          H_Poly;
	using SH_Poly      = typename P::         SH_Poly;

	using  H_PolyVert  = typename P::          H_PolyVert;
	using SH_PolyVert  = typename P::         SH_PolyVert;
	
	using  H_PolyEdge  = typename P::          H_PolyEdge;
	using SH_PolyEdge  = typename P::         SH_PolyEdge;
	
	// using  H_Subvert   = typename P::          H_Subvert;
	// using SH_Subvert   = typename P::         SH_Subvert;


	static constexpr bool Has_Vert_Data      = P::Has_Vert_Data;
	static constexpr bool Has_Subvert_Data   = P::Has_Subvert_Data;
	static constexpr bool Has_Poly_Data      = P::Has_Poly_Data;
	static constexpr bool Has_PolyVert_Data  = P::Has_PolyVert_Data;
	static constexpr bool Has_PolyEdge_Data  = P::Has_PolyEdge_Data;

	static constexpr bool Has_Edge_Links      = P::Has_Edge_Links;
	static constexpr bool Has_Vert_Poly_Links = P::Has_Vert_Poly_Links;


private:
	typename P::Verts _vs;
	typename P::Polys _ps;

	friend P;


	//
	// accessors from handles
	//
public:
	auto vert(typename P::IDX_Vert handle)       {  return P::Verts_Context::template create_accessor<MUTAB>(this, handle);  }
	auto vert(typename P::IDX_Vert handle) const {  return P::Verts_Context::template create_accessor<MUTAB>(this, handle);  }

	auto poly(typename P::IDX_Poly handle)       {  return P::Polys_Context::template create_accessor<MUTAB>(*this, handle);  }
	auto poly(typename P::IDX_Poly handle) const {  return P::Polys_Context::template create_accessor<CONST>(*this, handle);  }



	auto operator()(H_Vert handle)       {  return P::Verts_Context::template create_accessor<MUTAB>(this, handle);  }
	auto operator()(H_Vert handle) const {  return P::Verts_Context::template create_accessor<CONST>(this, handle);  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_Vert_Data)>
	auto& operator[](H_Vert handle)       {  return _vs[handle].data;  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_Vert_Data)>
	auto& operator[](H_Vert handle) const {  return _vs[handle].data;  }




	auto operator()(H_Poly handle)       {  return P::Polys_Context::template create_accessor<MUTAB>(this, handle);  }
	auto operator()(H_Poly handle) const {  return P::Polys_Context::template create_accessor<CONST>(this, handle);  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_Poly_Data)>
	auto& operator[](H_Poly handle)       {  return P::raw(*this, handle).data;  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_Poly_Data)>
	auto& operator[](H_Poly handle) const {  return P::raw(*this, handle).data;  }




	auto operator()(H_PolyVert handle)       {  return typename P::template A_Poly_PolyVert<MUTAB>(this, handle);  }
	auto operator()(H_PolyVert handle) const {  return typename P::template A_Poly_PolyVert<CONST>(this, handle);  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_PolyVert_Data)>
	auto& operator[](H_PolyVert handle)       {  return typename P::template A_Poly_PolyVert<MUTAB>(this, handle).data();  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_PolyVert_Data)>
	auto& operator[](H_PolyVert handle) const {  return typename P::template A_Poly_PolyVert<CONST>(this, handle).data();  }





	auto operator()(H_PolyEdge handle)       {  return P::PolyEdges_Context::template create_accessor<MUTAB>(this, handle);  }
	auto operator()(H_PolyEdge handle) const {  return P::PolyEdges_Context::template create_accessor<CONST>(this, handle);  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_PolyEdge_Data)>
	auto& operator[](H_PolyEdge handle)       {  return raw(*this, handle).data;  }

	template<class PP=P, SALGO_REQUIRES(PP::Has_PolyEdge_Data)>
	auto& operator[](H_PolyEdge handle) const {  return raw(*this, handle).data;  }



public:
	auto verts()       {  return P::template create_verts_accessor<MUTAB>(*this);  }
	auto verts() const {  return P::template create_verts_accessor<CONST>(*this);  }

	auto polys()       {  return P::template create_polys_accessor<MUTAB>(*this);  }
	auto polys() const {  return P::template create_polys_accessor<CONST>(*this);  }

public:
	Mesh() = default;
	Mesh(const Mesh&) = default;
	Mesh(Mesh&&) = default;

	template<class OP>
	Mesh(const Mesh<OP>& o) {
		this->append(o);
	}

public:
	void compact() {
		if constexpr(P::Verts_Erasable_Mode == ERASABLE_HOLES) {
			Dynamic_Array<int> v_remap( verts().domain() );
			_vs.compact([&](auto&& old, auto&& neww) {
				v_remap[old] = neww;
			});

			for(auto& p : _ps) {
				for(int i=0; i<3; ++i) p->verts[i].vert = (H_Vert) v_remap[ p->verts[i].vert ];
			}
		}

		// if constexpr(P::Polys_Erasable_Mode == ERASABLE_HOLES) {
		// 	Dynamic_Array<int> p_remap( polys().domain() );
		// 	_ps.compact([&](auto&& old, auto&& neww) {
		// 		p_remap[old] = neww;
		// 	});

		// 	if constexpr(P::Has_Vert_Poly_Links) {
		// 		for(auto& v : _vs) {
		// 			decltype(v->poly_links) new_ht;

		// 			for(auto& link : v->poly_links) {
		// 				new_ht.emplace(H_PolyVert{ (H_Poly) p_remap[ link->poly ], link->ith });
		// 			}

		// 			v->poly_links = std::move(new_ht);
		// 		}
		// 	}

		// 	if constexpr(P::Has_Edge_Links) {
		// 		for(auto& p : _ps) {
		// 			for(auto& pe : p->edges) {
		// 				if(pe.link.valid()) pe.link.poly = (H_Poly) p_remap[ pe.link.poly ];
		// 			}
		// 		}
		// 	}
		// }
	}

}; // class Mesh









template<class P>
struct With_Builder : Mesh<P> {
	using Mesh<P>::Mesh;

	using typename P::Scalar;
	using typename P::Vert_Data;
	using typename P::Subvert_Data;
	using typename P::Poly_Data;
	using typename P::PolyVert_Data;
	using typename P::PolyEdge_Data;
	using P::Verts_Erasable_Mode;
	using P::Polys_Erasable_Mode;
	using P::Has_Edge_Links;
	using P::Has_Vert_Poly_Links;

	template<class X>
	using VERT_DATA = With_Builder< Params<
		Scalar, X, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
		Has_Edge_Links, Has_Vert_Poly_Links>>;

	template<class X>
	using SUBVERT_DATA = With_Builder< Params<
		Scalar, Vert_Data, X,
		Poly_Data, PolyVert_Data, PolyEdge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
		Has_Edge_Links, Has_Vert_Poly_Links>>;

	template<class X>
	using POLY_DATA = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		X, PolyVert_Data, PolyEdge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
		Has_Edge_Links, Has_Vert_Poly_Links>>;

	template<class X>
	using POLYVERT_DATA = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, X, PolyEdge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
		Has_Edge_Links, Has_Vert_Poly_Links>>;

	template<class X>
	using POLYEDGE_DATA = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, X, Verts_Erasable_Mode, Polys_Erasable_Mode,
		Has_Edge_Links, Has_Vert_Poly_Links>>;


	using VERTS_ERASABLE = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, ERASABLE_HOLES, Polys_Erasable_Mode,
		Has_Edge_Links, Has_Vert_Poly_Links>>;

	using POLYS_ERASABLE = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, Verts_Erasable_Mode, ERASABLE_HOLES,
		Has_Edge_Links, Has_Vert_Poly_Links>>;


	using VERTS_ERASABLE_REORDER = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, ERASABLE_REORDER, Polys_Erasable_Mode,
		Has_Edge_Links, Has_Vert_Poly_Links>>;

	using POLYS_ERASABLE_REORDER = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, Verts_Erasable_Mode, ERASABLE_REORDER,
		Has_Edge_Links, Has_Vert_Poly_Links>>;


	using EDGE_LINKS = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
		true, Has_Vert_Poly_Links>>;

	using VERT_POLY_LINKS = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
		Has_Edge_Links, true>>;

	using FULL = With_Builder< Params<
		Scalar, Vert_Data, Subvert_Data,
		Poly_Data, PolyVert_Data, PolyEdge_Data, ERASABLE_HOLES, ERASABLE_HOLES,
		true, true>>;
};




} // namespace salgo::geom::g3d::_::mesh











namespace salgo::geom::g3d {



template< class T = double >
using Mesh = typename _::mesh::With_Builder< _::mesh::Params<
	T, // SCALAR
	void,   // VERT_DATA
	void,   // SUBVERT_DATA
	void,   // POLY_DATA
	void,   // POLYVERT_DATA
	void,   // POLYEDGE_DATA
	_::mesh::NOT_ERASABLE,  // VERTS_ERASABLE
	_::mesh::NOT_ERASABLE,  // POLYS_ERASABLE
	false,  // EDGE_LINKS
	false   // POLYVERT_LINKS
>>;




} // namespace salgo::geom::g3d

