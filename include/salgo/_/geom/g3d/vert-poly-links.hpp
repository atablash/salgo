#pragma once



#include <unordered_set>

namespace salgo::geom::g3d {




template<class MESH>
bool has_valid_vert_poly_links(MESH& mesh) {
	using H_PolyVert = typename MESH::H_PolyVert;
	std::unordered_set< H_PolyVert, salgo::Hash<H_PolyVert> > checked; // TODO: replace with salgo::Hash_Table

	for(auto& v : mesh.verts()) {
		for(auto& vp : v.vertPolys()) {
			auto pv = vp.polyVert();
			if(v != pv.vert()) return false;

			bool inserted = checked.insert( pv.handle() ).second;
			if(!inserted) {
				// some vertex already linked to this poly_vert
				return false;
			}
		}
	}

	for(auto& p : mesh.polys()) {
		for(auto& pv : p.polyVerts()) {
			auto it = checked.find( pv.handle() );
			if(it == checked.end()) {
				// this poly_vert is not pointed by its vert
				return false;
			}
		}
	}

	return true;
}












/* not used - mesh object automatically adds links when adding polys

template<class MESH>
void compute_vert_poly_links(MESH& mesh) {

	for(auto v : mesh.verts) {
		DCHECK(v.poly_links.is_empty()) << "compute_plinks expects empty plinks";
	}

	for(auto p : mesh.polys) {
		for(auto pv : p.verts) {
			pv.vert.poly_links.add( pv );
		}
	}
}
*/



} // namespace salgo::geom::g3d
