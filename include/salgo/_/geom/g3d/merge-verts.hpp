#pragma once

#include "named-arguments.hpp"

namespace salgo::geom::g3d {




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
// if links are present, this effectively collapses edge
//
NAMED_ARGUMENT(ALPHA)
NAMED_ARGUMENT(FORCE_B_INTO_A)

template<class VERT, class... ARGS>
void merge_verts(VERT a, VERT b, ARGS&&... _args) {
	auto args = Named_Arguments( std::forward<ARGS>(_args)... );

	auto alpha = args(ALPHA, 0.5);
	auto force_b_into_a = args(FORCE_B_INTO_A, false);

	static_assert(VERT::Mesh::Polys_Erasable, "merge_verts requires POLYS_ERASABLE");
	static_assert(VERT::Mesh::Has_Vert_Poly_Links, "merge_verts requires VERT_POLY_LINKS");

	// this is optimization, as well as bugfix - without it program will crash in some cases,
	// because 'a' could become isolated and deleted while this function is still running
	// (edge-collapse)
	if constexpr(VERT::Mesh::Has_Vert_Poly_Links) {
		if(!force_b_into_a) {
			if(a.vertPolys().count() < b.vertPolys().count()) {
				std::swap(a, b);
				alpha = 1.0 - alpha;
			}
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
			vp.polyVert().change_vert( a ); // invalidates `vp`

			erase_poly_if_degenerate( p,
				ERASE_ISOLATED_VERTS = true,
				ON_VERT_ERASE = args,
				ON_POLY_ERASE = args
			);
		}
	}

	b.fast_erase(
		ON_VERT_ERASE = args
	);

	// auto r = check_solid(a.mesh(), Check_Solid_Flags::ALLOW_HOLES);
	// if(!r.is_solid) {
	// 	std::cout << "error " << (int)r.failure << std::endl;
	// 	exit(1);
	// }
}








} // namespace salgo
