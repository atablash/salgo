#pragma once

#include "named-arguments.hpp"


namespace salgo::geom::g3d {




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


template<class POLY>
auto get_longest_polyEdge(const POLY& poly) {
	using Scalar = typename POLY::Scalar;
	using Poly_Edge = std::remove_reference_t< decltype(poly.polyEdge(0)) >;

	Scalar best = -1;
	Poly_Edge best_pe;

	for(auto& pe : poly.polyEdges()) {
		Scalar cand = pe.squared_length();
		if(cand > best) {
			best = cand;
			best_pe = pe;
		}
	}

	return best_pe;
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





template<class POLY, class... ARGS>
void erase_poly_if_degenerate(POLY poly, ARGS&&... _args) {
	auto args = Named_Arguments( std::forward<ARGS>(_args)... );

	const auto erase_isolated_verts = args(ERASE_ISOLATED_VERTS, false);

	for(auto& pv : poly.polyVerts()) {
		auto pw = pv.next();

		auto v = pv.vert();
		auto w = pw.vert();

		if(v != w) continue;

		// yes, remove

		// call ON_POLY_ERASE callback here (not inside poly.erase()), while edge-links are still valid
		if constexpr (args.has(ON_POLY_ERASE)) {
			args(ON_POLY_ERASE)(poly);
		}

		if constexpr(POLY::Mesh::Has_Edge_Links) {
			pv.next_polyEdge().unlink_if_linked();

			// note: in some corner-cases poly can be edge-linked to itself
			// it's generally not good situation to work with, but this function should support this anyway

			auto vv = pv.prev_polyEdge().linked_polyEdge();
			if(vv.valid()) vv.unlink();

			auto ww = pw.next_polyEdge().linked_polyEdge();
			if(ww.valid()) ww.unlink();

			if(vv.valid() && ww.valid()) {
				vv.link(ww);
				if constexpr(args.has(ON_EDGE_LINKED)) {
					args(ON_EDGE_LINKED)(vv);
				}
			}
		}

		poly.erase(
			UNLINK_EDGE_LINKS = false,
			ERASE_ISOLATED_VERTS = erase_isolated_verts,
			ON_VERT_ERASE = args
			// note: don't call ON_POLY_ERASE HERE, we did this already
		);

		break;
	}
}


template<class MESH>
void erase_isolated_verts(MESH& mesh) {
	if constexpr(MESH::Has_Vert_Poly_Links) {
		for(auto& v : mesh.verts()) {
			if(v.vertPolys().is_empty()) v.fast_erase();
		}
	}
	else {
		//std::cout << "not implemented!" << std::endl;
		abort();
		//throw std::runtime_error("not implemented!");
	}
}

template<class MESH>
bool has_isolated_verts(const MESH& mesh) {
	if constexpr(MESH::Has_Vert_Poly_Links) {
		for(auto& v : mesh.verts()) {
			if(v.vertPolys().is_empty()) return true;
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






template<class MESH>
void move_verts(MESH& mesh, const typename MESH::Vector& d) {
    for(auto& v : mesh.verts()) v.pos() += d;
}


template<class MESH>
void invert_polys(MESH& mesh) {
    for(auto& p : mesh.polys()) {
        auto a = p.vert(1);
        auto b = p.vert(2);

        p.polyVert(1).change_vert(b);
        p.polyVert(2).change_vert(a);
    }
}


template<class MESH, class OTHER>
void append(MESH& mesh, const OTHER& other) {
    using H_Vert       = typename MESH ::H_Vert;
    using H_Vert_Other = typename OTHER::H_Vert;

    mesh.verts().reserve( mesh.verts().domain() + other.verts().count() );
    mesh.polys().reserve( mesh.polys().domain() + other.polys().count() );

    Hash_Table<H_Vert_Other, H_Vert> v_remap;
    v_remap.reserve( other.verts().count() );

    for(auto& v : other.verts()) {
        auto new_vert = mesh.verts().add( v.pos()[0], v.pos()[1], v.pos()[2] );
        if constexpr(MESH::Has_Vert_Data) new_vert.data() = v.data();

        v_remap.emplace( v, new_vert );
    }

    for(auto& p : other.polys()) {
        auto new_poly = mesh.polys().add(
            v_remap[ p.vert(0) ],
            v_remap[ p.vert(1) ],
            v_remap[ p.vert(2) ]
        );

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






} // namespace salgo::geom::g3d
