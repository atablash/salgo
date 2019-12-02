#pragma once


#include "mesh/mesh.inl"
#include "mesh-utils.hpp"

#include "../../list.hpp"
#include "../../hash-table.hpp"


#include <map>


namespace salgo::geom::geom_3d {





struct Cap_Hole_Result {
	int num_polys_created = 0;
};

//
// for triangle meshes
//
template<class EDGE>
Cap_Hole_Result cap_hole(const EDGE& edge) {
	Cap_Hole_Result r;

	auto& m = edge.mesh();

	using H_PolyEdge = typename EDGE::Mesh::H_PolyEdge;

	typename List< H_PolyEdge > ::COUNTABLE perimeter;

	// iterators to perimeter, ordered by score
	std::multimap<double, typename decltype(perimeter)::Handle_Small> cands; // todo: replace with salgo heap/tree

	//std::unordered_map<H_Poly_Edge, typename decltype(cands)::iterator> where_cands;
	Hash_Table<H_PolyEdge, typename decltype(cands)::iterator> where_cands;


	auto get_score = [](auto e0, auto e1){
		auto t0 = e0.trace();
		auto t1 = e1.trace();

		auto my_normal = t1.cross( t0 ).eval();
		my_normal.normalize();

		auto normal0 = compute_poly_normal( e0.poly() );
		auto normal1 = compute_poly_normal( e1.poly() );

		auto score0 = normal0.dot(my_normal) + 1;
		auto score1 = normal1.dot(my_normal) + 1;

		// shape score: area / perimeter
		double min_angle = std::min({
			compute_angle(-t0, t1),
			compute_angle(t0, t0+t1),
			compute_angle(-t1, -(t0+t1))
		});

		return score0 * score1 * min_angle;
	};


	auto add_cand = [&](const auto& h_perim_0) {
		auto perim_0 = perimeter( h_perim_0 );
		auto perim_1 = perim_0.next();
		if(perim_1.not_found()) perim_1 = perimeter(FIRST);

		auto v0 = m(perim_0);
		auto v1 = m(perim_1);

		DCHECK_EQ( v0.next_vert(), v1.prev_vert() ) << "verts must be adjacent";

		auto cands_it = cands.insert({get_score(v0, v1), perim_0});
		where_cands.emplace(v0, cands_it);

		//LOG(INFO) << "where_cands.insert(" << (*it) << ")";
	};


	auto del_cand = [&](const auto& e) {

		auto it = where_cands(e);

		DCHECK(it.found()) << "edge not found in where_cands";
		DCHECK(it.val() != cands.end()) << "edge not found in cands";

		cands.erase( it.val() );
		where_cands.erase(it);
	};


	auto he = edge.handle();
	do {
		while(m(he).next().is_linked()) {
			he = m(he).next().linked_polyEdge();
		}

		he = m(he).next();

		perimeter.emplace_back( he );
	} while(he != edge);


	for(auto& p : perimeter) {
		add_cand(p);
	}


	while(perimeter.count() >= 3) {
		auto iter = cands.end(); --iter;

		auto curr = perimeter( iter->second );

		//LOG(INFO) << "score: " << it->first;

		// get next edge in perimeter
		auto next = curr.next();
		if(next.not_found()) next = perimeter(FIRST);

		DCHECK_EQ( m(curr).next_vert(), m(next).prev_vert() ) << "edges not adjacent";

		auto i0 = m(curr).prev_vert().handle();
		auto i1 = m(next).next_vert().handle();
		auto i2 = m(curr).next_vert().handle();
		// add cand, cand+1 poly
		auto p = m.polys().add(i0, i1, i2);

		++r.num_polys_created;

		//LOG(INFO) << "add " << i0 << " " << i1 << " " << i2;

		// create missing edge-links
		m(curr).link( p.polyEdge(2) );
		m(next).link( p.polyEdge(1) );

		// get prev edge in perimeter
		auto prev = curr.prev();
		if(prev.not_found()) prev = perimeter(LAST);

		// insert new edge
		auto new_edge = curr.emplace_before(p.polyEdge(0));

		DCHECK_EQ( p.polyEdge(0).prev_vert(), m(prev).next_vert() ) << "new_edge not adjacent";
		DCHECK_EQ( p.polyEdge(0).next_vert(), m(next).next_vert() ) << "new_edge not adjacent";

		// erase 3 candidate polys from priority queue
		del_cand(m(prev));
		del_cand(m(next));
		del_cand(m(curr));

		// remove 2 old edges
		perimeter.erase(curr);
		perimeter.erase(next);

		// add 2 new candidate polys
		add_cand(prev);
		add_cand(new_edge);

		DCHECK_EQ(perimeter.count(), cands.size());
		DCHECK_EQ(where_cands.count(), cands.size());
	}

	// add last edge-link
	{
		auto it = perimeter.begin();
		auto e0 = *it;
		++it;
		auto e1 = *it;

		m(e0).link( m(e1) );
	}

	return r;
}





struct Cap_Holes_Result {
	int num_holes_capped = 0;
	int num_polys_created = 0;
};

template<class MESH>
Cap_Holes_Result cap_holes(MESH& mesh) {
	Cap_Holes_Result r;

	for(auto& p : mesh.polys()) {
		for(auto& pe : p.polyEdges()) {
			if(!pe.is_linked()) {
				auto lr = cap_hole(pe);
				++r.num_holes_capped;
				r.num_polys_created += lr.num_polys_created;
			}
		}
	}

	return r;
}






} // namespace salgo


