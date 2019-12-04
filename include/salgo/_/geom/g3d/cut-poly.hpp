#pragma once

namespace salgo::geom::g3d {





/*
	   C                 C
	  / \               /'\
     /   \             / ' \
    /     \    -->    /  '  \
   /       \         /   P   \
  /	        \       /. '   ' .\
 A-----------B     A-----------B

*/
//
// cut ABC at P (P should already exist)
//
// can create degenerate polys if P is any of {A,B,C}
//
template<class POLY, class VERT>
auto cut_poly(POLY abc, VERT p) {
	using EDGE = std::remove_reference_t< decltype( abc.polyEdge(0) ) >;

	DCHECK_NE(abc.vert(0), p);
	DCHECK_NE(abc.vert(1), p);
	DCHECK_NE(abc.vert(2), p);

	auto& mesh = abc.mesh();

	// DCHECK( is_solid(mesh) );

	struct Result {
		POLY old_abc;
		POLY new_abp, new_bcp, new_cap;

		EDGE old_bc, old_ca;
		EDGE new_bc, new_ca;
		EDGE new_ap, new_bp, new_cp;
	};
	Result r;

	auto a = abc.vert(0);
	auto b = abc.vert(1);
	auto c = abc.vert(2);

	r.new_abp = r.old_abc = abc;

	r.old_abc.polyVert(2).change_vert(p);

	r.new_bcp = mesh.polys().add(b,c,p);
	r.new_cap = mesh.polys().add(c,a,p);

	r.old_bc = r.old_abc.polyEdge(1);
	r.old_ca = r.old_abc.polyEdge(2);

	r.new_bc = r.new_bcp.polyEdge(0);
	r.new_ca = r.new_cap.polyEdge(0);

	if(r.old_bc.is_linked()) {
		auto other = r.old_bc.linked_polyEdge();
		r.old_bc.unlink();
		r.new_bc.link(other);
	}

	if(r.old_ca.is_linked()) {
		auto other = r.old_ca.linked_polyEdge();
		r.old_ca.unlink();
		r.new_ca.link(other);
	}

	r.new_abp.polyEdge(1).link( r.new_bcp.polyEdge(2) );
	r.new_bcp.polyEdge(1).link( r.new_cap.polyEdge(2) );
	r.new_cap.polyEdge(1).link( r.new_abp.polyEdge(2) );

	r.new_bp = r.new_abp.polyEdge(1);
	r.new_cp = r.new_bcp.polyEdge(1);
	r.new_ap = r.new_cap.polyEdge(1);

	// DCHECK( is_solid(mesh) );

	return r;
}






}
