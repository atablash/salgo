#pragma once






template<class V0, class V1>
auto compute_angle(const V0& v0, const V1& v1) {
	return acos(v0.dot(v1) / (v0.norm() * v1.norm()));
}


template<class POLY_VERT>
auto compute_poly_vert_angle(const POLY_VERT& pv) {
	auto v0 = pv.prev().vert().pos() - pv.vert().pos();
	auto v1 = pv.next().vert().pos() - pv.vert().pos();
	return compute_angle(v0, v1);
}






template<class POLY>
auto compute_poly_normal(POLY p) {
	auto v01 = p.vert(1).pos() - p.vert(0).pos();
	auto v02 = p.vert(2).pos() - p.vert(0).pos();
	auto normal = v01.cross(v02);
	normal.normalize();
	return normal;
}










// //
// // no test coverage
// //
// template< class MESH >
// std::vector<bool> compute_isolated_vertices( const MESH& mesh, bool use_vert_poly_links = false ) {

// 	if(use_vert_poly_links) {
// 		DCHECK(false) << "not implemented";
// 		abort();
// 		//throw "not implemented";
// 	}
// 	else {
// 		std::vector<bool> isolated( mesh.verts_domain(), true );

// 		for( auto p : mesh.polys() ) {
// 			for( auto pv : p.verts() ) {
// 				isolated[ pv.key() ] = false;
// 			}
// 		}

// 		return isolated;
// 	}
// }


// //
// // no test coverage
// //
// template< class MESH >
// bool has_isolated_vertices( const MESH& mesh, bool use_vert_poly_links = false ) {
// 	auto isolated = compute_isolated_vertices(mesh, use_vert_poly_links);

// 	for(const auto& b : isolated) {
// 		if(b) return true;
// 	}

// 	return false;
// }










// //
// // no test coverage
// //
// template< class MESH, class GET_V_NORMAL >
// void grow( MESH& mesh, double amount, const GET_V_NORMAL& get_v_normal ) {
	
// 	for(auto v : mesh.verts()) {
// 		v.pos() += amount * get_v_normal( v.key() );
// 	}
// }



// template< class MESH >
// void grow( MESH& mesh, double amount) {
// 	grow(mesh, amount, [&mesh](int iv){ return mesh.vert(iv).props().normal; });
// }





template< class POLY >
void remove_if_degenerate(POLY poly) {
	for(auto& v : poly.verts()) {
		auto w = v.next();

		if(v == w) {
			// yes, remove
			if constexpr(POLY::Mesh::Has_Edge_Links) {
				v.next_poly_edge().unlink_if_linked();

				// note: in some corner-cases poly can be edge-linked to itself
				// it's generally not good situation to work with, but this function should support this anyway

				auto vv = v.prev_poly_edge().linked_edge();
				if(vv.exists()) vv.unlink();

				auto ww = w.next_poly_edge().linked_edge();
				if(ww.exists()) ww.unlink();

				if(vv.exists() && ww.exists()) {
					vv.link(ww);
				}
			}

			poly.erase({
				.unlink_edge_links = false,
				.remove_isolated_verts = true,
			});

			break;
		}
	}
}


template<class MESH>
void remove_isolated_verts(MESH& mesh) {
	if constexpr(MESH::Has_Vert_Poly_Links) {
		for(auto& v : mesh.verts()) {
			if(v.polys().empty()) v.fast_erase();
		}
	}
	else {
		std::cout << "not implemented!" << std::endl;
		exit(1);
		//throw std::runtime_error("not implemented!");
	}
}

template<class MESH>
bool has_isolated_verts(const MESH& mesh) {
	if constexpr(MESH::Has_Vert_Poly_Links) {
		for(auto& v : mesh.verts()) {
			if(v.polys().empty()) return true;
		}
		return false;
	}
	else {
		std::vector<bool> vs( mesh.verts().domain() );

		for(auto& p : mesh.polys()) {
			for(int i=0; i<3; ++i) vs[ p.vert(i).handle() ] = true;
		}

		for(auto& v : mesh.verts()) if(!vs[ v.handle() ]) return false;
		return true;
	}
}

