#pragma once

namespace salgo {





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
template<class POLY_EDGE, class VERT>
void split_edge(POLY_EDGE ab, VERT p) {
	auto& mesh = ab.mesh();

	ab.next_polyVert().change_vert( p ); // B -> P

	auto b = ab.next_vert();
	auto c = ab.opposite_vert();
	auto pbc = mesh.polys().add(p, b, c);

	auto old_bc = ab.next();

	// unlink old_bc, link new_bc
	if(old_bc.has_link()) {
		auto cb = old_bc.linked_polyEdge();
		cb.unlink();
		auto new_bc = pbc.polyVert(0).opposite_polyEdge();
		cb.link( new_bc );
	}

	// link pc-cp
	auto pc = old_bc;
	auto cp = pbc.polyVert(0).prev_polyEdge();
	pc.link( cp );

	// process lower half of the image
	if( ab.has_link() ) {
		auto ba = ab.linked_polyEdge();
		ba.prev_polyVert().change_vert( p ); // B -> P

		auto d = ba.opposite_vert();
		auto pdb = mesh.polys().add(p, d, b);

		auto old_db = ba.prev();

		// unlink old_db, link new_db
		if(old_db.has_link()) {
			auto bd = old_db.linked_polyEdge();
			bd.unlink();
			auto new_db = pdb.polyVert(0).opposite_polyEdge();
			bd.link( new_db );
		}

		// link dp-pd
		auto dp = old_db;
		auto pd = pdb.polyVert(0).next_polyEdge();
		dp.link( pd );

		// link pb-bp
		auto pb = pbc.polyVert(0).next_polyEdge();
		auto bp = pdb.polyVert(0).prev_polyEdge();
		pb.link( bp );
	}
}






}
