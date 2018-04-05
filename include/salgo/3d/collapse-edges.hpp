#pragma once




/*
     C
    / \
   /   \
  A-----B
   \   /
    \ /
     D
*/
//
// merge 'b' into 'a'
//
// if links are present, this effectively collapses edge (if present)
//
template<class VERT>
void merge_verts(VERT a, VERT b, const typename VERT::Mesh::Scalar& alpha) {

	// LOG(INFO) << "merge_verts(" << a.idx << ", " << b.idx << ", alpha:" << alpha << ")";

	a.pos() = a.pos() * (1-alpha)  +  b.pos() * alpha;

	if constexpr(VERT::Mesh::Has_Vert_Props) {
		a.props() = a.props() * (1-alpha)  +  b.props() * alpha;
	}

	// update polygons containing 'b': replace 'b'->'a'
	if constexpr(VERT::Mesh::Has_Vert_Poly_Links) {

		// here, keep an eye for iterator invalidation!
		for(auto pv : b.poly_verts()) {
			pv.change_vert( a );
		}

		for(auto pv : b.poly_verts()) {

			// we got degenerate triangle
			if(pv.vert() == pv.next().vert()) {
				if(pv.prev_poly_edge().has_link() && pv.prev_poly_edge().prev().has_link()) {
					auto e0 = pv.prev_poly_edge().linked_edge();
					auto e1 = pv.prev_poly_edge().prev().linked_edge();
					e0.unlink();
					e1.unlink();
					if(e0.poly() != e1.poly()) e0.link(e1);
				}

				pv.poly().erase();
			}
			else if(pv.vert() == pv.prev().vert()) {
				if(pv.next_poly_edge().has_link() && pv.next_poly_edge().next().has_link()) {
					auto e0 = pv.next_poly_edge().linked_edge();
					auto e1 = pv.next_poly_edge().next().linked_edge();
					e0.unlink();
					e1.unlink();
					if(e0.poly() != e1.poly()) e0.link(e1);
				}

				pv.poly().erase();
			}
		}
	}

	b.erase();
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
			for(auto ab : p.poly_edges()) {
				if(!ab.has_link()) continue;

				auto ba = ab.linked_edge();

				if(ba.next_poly_vert().next().vert() == ab.next_poly_vert().next().vert()) {

					if(ab.next().has_link() && ba.prev().has_link()) {

						auto cb = ab.next().linked_edge();
						auto bd = ba.prev().linked_edge();

						if(ba.poly() != cb.poly()) {
							cb.unlink();
							bd.unlink();

							cb.link(bd);
						}
					}

					if(ab.prev().has_link() && ba.next().has_link()) {

						auto ac = ab.prev().linked_edge();
						auto da = ba.next().linked_edge();

						if(ab.poly() != da.poly()) {
							ac.unlink();
							da.unlink();

							ac.link(da);
						}
					}

					ab.poly().erase();
					ba.poly().erase();
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
	Fast_Collapse_Edges_Result r;

	bool change = true;

	while(change) {
		change = false;
		++r.num_passes;

		for(auto p : mesh.polys()) {
			for(auto pe : p.poly_edges()) {
				if(pe.segment().trace().squaredNorm() <= max_edge_length * max_edge_length) {

					auto weight_sum = get_v_weight( pe.prev_vert().handle() ) + get_v_weight( pe.next_vert().handle() );

					merge_verts(pe.prev_vert(), pe.next_vert(),
						(typename MESH::Scalar)get_v_weight(pe.next_vert().handle()) / weight_sum);
					++r.num_edges_collapsed;

					// if weights can be modified
					if constexpr(std::is_lvalue_reference_v<decltype(get_v_weight(0))>) {
						get_v_weight( pe.prev_vert().handle() ) += get_v_weight( pe.next_vert().handle() );
					}

					//if(r.num_edges_collapsed >= Dupa::get()) {
					//	return r;
					//}

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
	std::vector<int32_t> weights(mesh.verts_domain(), 1);
	return fast_collapse_edges(mesh, max_edge_length, [&weights](auto i) -> auto& { return weights[i]; });
}








