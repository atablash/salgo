#pragma once


#include "solid.hpp"


namespace salgo {




/*
      C                  C
     / \                 |
    /   \                |
X--A-----B--Y   -->   X--A--Y
    \   /                |
     \ /                 |
      D                  D
*/
//
// merge 'b' into 'a'
//
// if links are present, this effectively collapses edge
//
template<class VERT>
void merge_verts(VERT a, VERT b, const typename VERT::Mesh::Scalar& alpha) {
	static_assert(VERT::Mesh::Polys_Erasable, "merge_verts requires POLYS_ERASABLE");
	static_assert(VERT::Mesh::Has_Vert_Poly_Links, "merge_verts requires VERT_POLY_LINKS");


	// this is optimization, as well as bugfix - without it program will crash in some cases,
	// because 'a' could become isolated and deleted while this function is still running
	if constexpr(VERT::Mesh::Has_Vert_Poly_Links) {
		if(a.vertPolys().count() < b.vertPolys().count()) {
			merge_verts(b, a, 1.0 - alpha);
			return;
		}
	}

	// LOG(INFO) << "merge_verts(" << a.handle() << ", " << b.handle() << ", alpha:" << alpha << ")";

	a.pos() = a.pos() * (1-alpha)  +  b.pos() * alpha;

	if constexpr(VERT::Mesh::Has_Vert_Data) {
		a.data() = a.data() * (1-alpha)  +  b.data() * alpha;
	}

	// update polygons containing 'b': replace 'b'->'a'
	if constexpr(VERT::Mesh::Has_Vert_Poly_Links) {

		// std::cout << "have " << b.vert_polys().count() << " polygons to change vertex " << b.handle() << "->" << a.handle() << std::endl;
		// for(auto& vp : b.vert_polys()) {
		// 	std::cout << "-> " << vp.poly().handle() << std::endl;
		// }

		for(auto& vp : b.vertPolys()) {
			//std::cout << "--> " << vp.poly().handle() << std::endl;
			auto p = vp.poly();
			vp.change_vert( a ); // invalidates `vp`

			remove_if_degenerate( p ); // will delete vertices that become isolated
		}
	}

	b.fast_erase();

	// auto r = check_solid(a.mesh(), Check_Solid_Flags::ALLOW_HOLES);
	// if(!r.is_solid) {
	// 	std::cout << "error " << (int)r.failure << std::endl;
	// 	exit(1);
	// }
}









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
				if(!ab.has_link()) continue;

				auto ba = ab.linked_polyEdge();

				// C == D
				if(ba.opposite_vert() == ab.opposite_vert()) {

					if(ab.next().has_link() && ba.prev().has_link()) {

						auto cb = ab.next().linked_polyEdge();
						auto bd = ba.prev().linked_polyEdge();

						if(ba.poly().handle() != cb.poly().handle()) {
							cb.unlink();
							bd.unlink();

							cb.link(bd);
						}
					}

					if(ab.prev().has_link() && ba.next().has_link()) {

						auto ac = ab.prev().linked_polyEdge();
						auto da = ba.next().linked_polyEdge();

						if(ab.poly() != da.poly()) {
							ac.unlink();
							da.unlink();

							ac.link(da);
						}
					}

					ab.poly().erase({ .unlink_edge_links = false, .remove_isolated_verts = true }); // we handle edge-links ourselves
					ba.poly().erase({ .unlink_edge_links = false, .remove_isolated_verts = true });
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

					merge_verts(a, b, (typename MESH::Scalar) get_v_weight(a) / weight_sum);
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





