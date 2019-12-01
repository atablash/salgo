#pragma once

namespace salgo::geom::geom_3d {





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
	DCHECK_NE(abc.vert(0), p);
	DCHECK_NE(abc.vert(1), p);
	DCHECK_NE(abc.vert(2), p);

	struct Result {
		POLY old_abc;
		POLY new_abp, new_bcp, new_cap;
	};
	Result r;

	r.new_abp = r.old_abc = abc;

	r.old_abc.polyVert(2).change_vert(p);

	auto& mesh = abc.mesh();

	auto a = abc.vert(0);
	auto b = abc.vert(1);
	auto c = abc.vert(2);

	r.new_bcp = mesh.polys().add(b,c,p);
	r.new_cap = mesh.polys().add(c,a,p);

	auto old_bc = r.old_abc.polyEdge(1);
	auto old_ca = r.old_abc.polyEdge(2);

	auto new_bc = r.new_bcp.polyEdge(0);
	auto new_ca = r.new_cap.polyEdge(0);

	if(old_bc.is_linked()) {
		auto other = old_bc.linked_polyEdge();
		old_bc.unlink();
		new_bc.link(other);
	}

	if(old_ca.is_linked()) {
		auto other = old_ca.linked_polyEdge();
		old_ca.unlink();
		new_ca.link(other);
	}

	r.new_abp.polyEdge(1).link( r.new_bcp.polyEdge(2) );
	r.new_bcp.polyEdge(1).link( r.new_cap.polyEdge(2) );
	r.new_cap.polyEdge(1).link( r.new_abp.polyEdge(2) );

	return r;
}






}
