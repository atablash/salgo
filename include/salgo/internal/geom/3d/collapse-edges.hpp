#pragma once


#include "solid.hpp"
#include "merge-verts.hpp"


namespace salgo::geom::geom_3d {





/*
     C
    / \
   /   \
  A-----B
   \   /
    \ /
     D
*/

struct Clean_Flat_Surfaces_On_Edges_Result {
	int num_passes = 0;
	int num_polys_removed = 0;
};

template<class MESH>
auto clean_flat_surfaces_on_edges(MESH& mesh) {

	Clean_Flat_Surfaces_On_Edges_Result r;

	bool change = true;

	while(change) {
		++r.num_passes;
		change = false;

		for(auto p : mesh.polys()) {
			for(auto ab : p.polyEdges()) {
				if(!ab.is_linked()) continue;

				auto ba = ab.linked_polyEdge();

				// C == D
				if(ba.opposite_vert() == ab.opposite_vert()) {

					if(ab.next().is_linked() && ba.prev().is_linked()) {

						auto cb = ab.next().linked_polyEdge();
						auto bd = ba.prev().linked_polyEdge();

						if(ba.poly().handle() != cb.poly().handle()) {
							cb.unlink();
							bd.unlink();

							cb.link(bd);
						}
					}

					if(ab.prev().is_linked() && ba.next().is_linked()) {

						auto ac = ab.prev().linked_polyEdge();
						auto da = ba.next().linked_polyEdge();

						if(ab.poly() != da.poly()) {
							ac.unlink();
							da.unlink();

							ac.link(da);
						}
					}

					ab.poly().erase( UNLINK_EDGE_LINKS = false, ERASE_ISOLATED_VERTS = true ); // we handle edge-links ourselves
					ba.poly().erase( UNLINK_EDGE_LINKS = false, ERASE_ISOLATED_VERTS = true );
					r.num_polys_removed += 2;
					change = true;
					break; // skip the rest edges of this poly (it's removed and invalid now)
				}
			}
		}
	}

	return r;
}






struct Fast_Collapse_Edges_Result {
	int num_edges_collapsed = 0;
	int num_passes = 0;
};

//
// it's good to call clean_flat_surfaces_on_edges after this
//
template<class MESH, class GET_V_WEIGHT>
auto fast_collapse_edges(MESH& mesh, const typename MESH::Scalar& max_edge_length, const GET_V_WEIGHT& get_v_weight) {
	static_assert(MESH::Polys_Erasable, "merge_verts requires POLYS_ERASABLE");
	static_assert(MESH::Verts_Erasable, "merge_verts requires VERTS_ERASABLE");
	static_assert(MESH::Has_Vert_Poly_Links, "merge_verts requires VERT_POLY_LINKS");

	Fast_Collapse_Edges_Result r;

	bool change = true;

	// int brk = 0;

	while(change) {
		change = false;
		++r.num_passes;

		for(auto& p : mesh.polys()) {
			//if(brk++ >= 0) break;
			// std::cout << "brk " << brk << std::endl;

			// std::cout << "process poly " << p.handle() << std::endl;

			for(auto& pe : p.polyEdges()) {
				if(pe.segment().trace().squaredNorm() <= max_edge_length * max_edge_length) {
					// std::cout << "collapsing edge with length " << pe.segment().trace().norm() << std::endl;

					auto a = pe.prev_vert();
					auto b = pe.next_vert();

					auto weight_sum = get_v_weight( a ) + get_v_weight( b );

					merge_verts(a, b,
						ALPHA = (typename MESH::Scalar) get_v_weight(a) / weight_sum
					);
					// merge_verts(a, b, {
					// 	.alpha=
					// });
					++r.num_edges_collapsed;

					// poly does not exist anymore - watch out!

					// if weights can be modified
					if constexpr(std::is_lvalue_reference_v<decltype(get_v_weight(0))>) {
						get_v_weight( a ) += get_v_weight( b );
					}

					change = true;
					break; // this poly does not exist now, so break!
				}
			}
		}
	}

	return r;
}


template<class MESH>
auto fast_collapse_edges(MESH& mesh, const typename MESH::Scalar& max_edge_length) {
	std::vector<int32_t> weights(mesh.verts().domain(), 1);
	return fast_collapse_edges(mesh, max_edge_length, [&weights](auto i) -> auto& { return weights[(typename MESH::H_Vert) i]; });
}




} // namespace salgo





