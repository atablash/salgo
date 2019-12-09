#pragma once

#include "../solid.inl"

#include "../../../hash-table.inl"

#include "../../../named-arguments.hpp"
#include "../../../crtp.hpp"

namespace salgo::geom::g3d {


NAMED_ARGUMENT(INVERT_POLYS);


// does not transfer edge-links!
template<class MESH, class OTHER, class... ARGS>
void mesh_append_inplace(MESH& mesh, const OTHER& other, ARGS&&... _args) {
	using H_Vert       = typename MESH ::H_Vert;
	using H_Vert_Other = typename OTHER::H_Vert;

	auto args = Named_Arguments{ std::forward<ARGS>(_args)... };
	auto invert_polys = args(INVERT_POLYS, false);

	mesh.verts().reserve( mesh.verts().domain() + other.verts().count() );
	mesh.polys().reserve( mesh.polys().domain() + other.polys().count() );

	Hash_Table<H_Vert_Other, H_Vert> v_remap; // todo: replace with Dynamic_Array and compare performance
	v_remap.reserve( other.verts().count() );

	for(auto& v : other.verts()) {
		auto new_vert = mesh.verts().add( v.pos()[0], v.pos()[1], v.pos()[2] );
		if constexpr(MESH::Has_Vert_Data) new_vert.data() = v.data();

		v_remap.emplace( v, new_vert );
	}

	for(auto& p : other.polys()) {
		using A_Poly = decltype(mesh.polys().add(0,1,2));
		A_Poly new_poly;

		if(invert_polys) {
			new_poly = mesh.polys().add(
				v_remap[ p.vert(0) ],
				v_remap[ p.vert(2) ],
				v_remap[ p.vert(1) ]
			);
		}
		else {
			new_poly = mesh.polys().add(
				v_remap[ p.vert(0) ],
				v_remap[ p.vert(1) ],
				v_remap[ p.vert(2) ]
			);
		}

		(void)new_poly; // unused warning

		if constexpr(MESH::Has_Poly_Data) new_poly.data() = p.data();

		if constexpr(MESH::Has_PolyVert_Data) {
			for(int i=0; i<3; ++i) {
				new_poly.poly_vert(i).data() = p.poly_vert(i).data();
			}
		}

		if constexpr(MESH::Has_PolyEdge_Data) {
			for(int i=0; i<3; ++i) {
				new_poly.poly_edge(i).data() = p.poly_edge(i).data();
			}
		}
	}
}



template<class CRTP>
struct Mesh_Base {

	// Mesh_Base() = default;
	// Mesh_Base(Mesh_Base&&) = default;

	// defining constructors here makes them to be called before Derived is constructed... not much sense
	//
	// template<class O>
	// Mesh_Base(const Mesh_Base<O>& o) {
	// 	_self().append( *static_cast<const O*>(&o) );
	// }

	// does not transfer edge-links!
	template<class OTHER, class... ARGS>
	auto& append(OTHER&& other, ARGS&&... args) {
		mesh_append_inplace( _self(), std::forward<OTHER>(other), std::forward<ARGS>(args)... );
		return _self();
	}

	template<class TRANSFORM>
	auto& operator*=(TRANSFORM&& t) {
		for(auto& v : _self().verts()) {
			v.pos() = t * v.pos();
		}
		return _self();
	}

	bool is_solid() const {
		return g3d::is_solid( _self() );
	}

	SALGO_CRTP_COMMON(CRTP)
};


} // namespace salgo::geom::g3d

