#pragma once


#include "mesh.hpp"
#include "mesh-utils.hpp"



#include <list>
#include <map>


namespace salgo {





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

	using H_Poly_Edge = typename EDGE::Mesh::H_Poly_Edge;

	std::list< H_Poly_Edge > perimeter;

	// iterators to perimeter, ordered by score
	std::multimap<double, typename decltype(perimeter)::iterator> cands;

	//std::unordered_map<H_Poly_Edge, typename decltype(cands)::iterator> where_cands;
	salgo::Hash_Table<H_Poly_Edge, typename decltype(cands)::iterator> where_cands;


	auto get_score = [](auto e0, auto e1){
		auto t0 = e0.segment().trace();
		auto t1 = e1.segment().trace();

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


	auto add_cand = [&](const auto& it) {

		auto next = it;
		++next;
		if(next == perimeter.end()) next = perimeter.begin();

		DCHECK( m(*it).next_vert() == m(*next).prev_vert()) << "edges not adjacent";

		auto cands_it = cands.insert({get_score(m(*it), m(*next)), it});
		where_cands.emplace(*it, cands_it);

		//LOG(INFO) << "where_cands.insert(" << (*it) << ")";
	};


	auto del_cand = [&](const auto& e) {

		auto it = where_cands(e);

		DCHECK(it.exists()) << "edge not found in where_cands";
		DCHECK(it.val() != cands.end()) << "edge not found in cands";

		cands.erase( it.val() );
		where_cands.erase(it);
	};


	auto he = edge.handle();
	do {
		while(m(he).next().has_link()) {
			he = m(he).next().linked_edge();
		}

		he = m(he).next();

		perimeter.push_back(he);
	} while(he != edge);


	for(typename decltype(perimeter)::iterator iter = perimeter.begin(); iter != perimeter.end(); ++iter) {
		add_cand(iter);
	}


	while(perimeter.size() >= 3) {
		auto iter = cands.end(); --iter;
		auto curr = iter->second;

		//LOG(INFO) << "score: " << it->first;

		// get next edge in perimeter
		auto next = curr; ++next;
		if(next == perimeter.end()) next = perimeter.begin();

		DCHECK( m(*curr).next_vert() == m(*next).prev_vert()) << "edges not adjacent";

		auto i0 = m(*curr).prev_vert().handle();
		auto i1 = m(*next).next_vert().handle();
		auto i2 = m(*curr).next_vert().handle();
		// add cand, cand+1 poly
		auto p = m.polys_add(i0, i1, i2);

		++r.num_polys_created;

		//LOG(INFO) << "add " << i0 << " " << i1 << " " << i2;

		// create missing edge-links
		m(*curr).link( p.poly_edge(2) );
		m(*next).link( p.poly_edge(1) );

		// get prev edge in perimeter
		auto prev = curr;
		if(prev == perimeter.begin()) prev = perimeter.end();
		--prev;

		// insert new edge
		auto new_edge = perimeter.insert( curr, p.poly_edge(0).handle() );

		DCHECK_EQ( p.poly_edge(0).prev_vert().handle(), m(*prev).next_vert().handle() ) << "new_edge not adjacent";
		DCHECK_EQ( p.poly_edge(0).next_vert().handle(), m(*next).next_vert().handle() ) << "new_edge not adjacent";

		// erase 3 candidate polys from priority queue
		del_cand(*prev);
		del_cand(*next);
		del_cand(*iter->second);

		// remove 2 old edges
		perimeter.erase(curr);
		perimeter.erase(next);

		// add 2 new candidate polys
		add_cand(prev);
		add_cand(new_edge);

		DCHECK_EQ(perimeter.size(), cands.size());
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

	for(auto p : mesh.polys()) {
		for(auto pe : p.poly_edges()) {
			if(!pe.has_link()) {
				auto lr = cap_hole(pe);
				++r.num_holes_capped;
				r.num_polys_created += lr.num_polys_created;
			}
		}
	}

	return r;
}






} // namespace salgo


