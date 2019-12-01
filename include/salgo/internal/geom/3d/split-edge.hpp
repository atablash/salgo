#pragma once

namespace salgo::geom::geom_3d {





/*
	 C               C
	/ \             /|\
   /   \           / | \
  A-----B   -->   A--P--B
   \   /           \ | /
	\ /             \|/
	 D               D
*/
//
// split AB at P (P should already exist)
//
// can create degenerate polys if P is any of {A,B,C,D}
//
template<class POLY_EDGE, class VERT>
auto split_edge(POLY_EDGE ab, VERT p) {
	DCHECK_NE(ab.prev_vert(), ab.next_vert());
	DCHECK_NE(ab.prev_vert(), p);
	DCHECK_NE(ab.next_vert(), p);

	using POLY = std::remove_reference_t< decltype(ab.poly()) >;

	struct Result {
		// poly-edges removed
		POLY_EDGE old_ab;

		// poly-edges renamed
		POLY_EDGE old_bc, new_bc;
		POLY_EDGE old_db, new_db;

		// poly-edges added
		POLY_EDGE new_ap, new_pb, new_dp, new_pc;

		// polys removed
		POLY old_abc, old_adb;

		// polys added
		POLY new_apc, new_pbc, new_adp, new_pdb;
	};
	Result r;

	r.old_ab = ab;
	r.new_ap = ab;

	r.new_apc = r.old_abc = ab.poly();

	auto& mesh = ab.mesh();

	auto b = ab.next_vert();
	auto c = ab.opposite_vert();
	r.new_pbc = mesh.polys().add(p, b, c);

	r.old_bc = ab.next();

	ab.next_polyVert().change_vert( p ); // B -> P

	// unlink old_bc, link new_bc
	if(r.old_bc.is_linked()) {
		auto cb = r.old_bc.linked_polyEdge();
		cb.unlink();
		r.new_bc = r.new_pbc.polyVert(0).opposite_polyEdge();
		cb.link( r.new_bc );
	}

	// link pc-cp
	r.new_pc = r.old_bc;
	auto cp = r.new_pbc.polyVert(0).prev_polyEdge();
	r.new_pc.link( cp );

	// process lower half of the image
	if( ab.is_linked() ) {
		auto ba = ab.linked_polyEdge();

		r.new_adp = r.old_adb = ba.poly();

		auto d = ba.opposite_vert();
		r.new_pdb = mesh.polys().add(p, d, b);

		r.old_db = ba.prev();

		ba.prev_polyVert().change_vert( p ); // B -> P

		// unlink old_db, link new_db
		if(r.old_db.is_linked()) {
			auto bd = r.old_db.linked_polyEdge();
			bd.unlink();
			r.new_db = r.new_pdb.polyVert(0).opposite_polyEdge();
			bd.link( r.new_db );
		}

		// link dp-pd
		r.new_dp = r.old_db;
		auto pd = r.new_pdb.polyVert(0).next_polyEdge();
		r.new_dp.link( pd );

		// link pb-bp
		r.new_pb = r.new_pbc.polyVert(0).next_polyEdge();
		auto bp = r.new_pdb.polyVert(0).prev_polyEdge();
		r.new_pb.link( bp );
	}

	return r;
}






}
