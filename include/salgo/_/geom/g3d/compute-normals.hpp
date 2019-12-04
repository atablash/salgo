#pragma once

#include "mesh-utils.hpp"

#include <vector>


namespace salgo::geom::g3d {



//
// compute_normals - fast version, no weighting
//
template< class MESH, class GET_V_NORMAL >
void fast_compute_vert_normals( MESH& mesh,
		const GET_V_NORMAL& get_v_normal ) {

	std::vector<int> nums( mesh.verts().domain() );

	for(auto& v : mesh.verts()) {
		// DLOG(INFO) << "v";
		get_v_normal( v.handle() ) = {0,0,0};
	}

	for(auto& p : mesh.polys()) {
		// DLOG(INFO) << "p";
		auto v01 = p.vert(1).pos() - p.vert(0).pos();
		auto v02 = p.vert(2).pos() - p.vert(0).pos();
		auto normal = v01.cross(v02);
		normal.normalize();

		for(auto& pv : p.polyVerts()) {
			get_v_normal( pv.vert().handle() ) += normal;
			++nums[ pv.vert().handle() ];
		}
	}

	for(auto& v : mesh.verts()) {
		if(nums[ v.handle() ] > 0) {
			get_v_normal( v.handle() ) /= nums[ v.handle() ];
			get_v_normal( v.handle() ).normalize();
		}
	}
}



template<class MESH>
void fast_compute_vert_normals( MESH& mesh ) {
	fast_compute_vert_normals( mesh, [&mesh](int iv) -> auto& { return mesh.vert(iv).data().normal; } );
}


















//
// compute normals - slower version with weighting
//
template< class MESH, class GET_V_NORMAL >
void compute_vert_normals( MESH& mesh,
		const GET_V_NORMAL& get_v_normal ) {

	std::vector<typename MESH::Scalar> weights( mesh.verts().domain() );

	for(auto v : mesh.verts()) {
		get_v_normal( v.handle() ) = {0,0,0};
	}

	for(auto p : mesh.polys()) {
		auto normal = compute_poly_normal(p);

		for(auto pv : p.polyVerts()) {
			auto angle = compute_poly_vert_angle( pv );
			get_v_normal( pv.vert().handle() ) += normal * angle;
			weights[ pv.vert().handle() ] += angle;
		}
	}

	for(auto v : mesh.verts()) {
		if(weights[ v.handle() ] > 0) {
			get_v_normal( v.handle() ) /= weights[ v.handle() ];
			get_v_normal( v.handle() ).normalize();
		}
	}
}



template<class MESH>
void compute_vert_normals( MESH& mesh ) {
	compute_vert_normals( mesh, [&mesh](int iv) -> auto& { return mesh.vert(iv).data().normal; } );
}





}



