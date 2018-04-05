#pragma once



#include <unordered_set>






template<class MESH>
bool has_valid_vert_poly_links(MESH& mesh) {
	std::unordered_set< typename MESH::H_Poly_Vert > checked;

	for(auto v : mesh.verts()) {
		for(auto pv : v.poly_verts()) {

			if(v != pv.vert()) return false;

			bool inserted = checked.insert( pv.handle() ).second;
			if(!inserted) {
				// some vertex already linked to this poly_vert
				return false;
			}
		}
	}

	for(auto p : mesh.polys()) {
		for(auto pv : p.poly_verts()) {
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
		DCHECK(v.poly_links.empty()) << "compute_plinks expects empty plinks";
	}

	for(auto p : mesh.polys) {
		for(auto pv : p.verts) {
			pv.vert.poly_links.add( pv );
		}
	}
}
*/


