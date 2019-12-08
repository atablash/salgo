#pragma once

#include "../../list.inl"

#include <algorithm> // std::sort

namespace salgo::geom::g3d {



// can break edge-links
template<class MESH>
void remove_2sided_and_double(MESH& mesh) {
	using SH_Vert = typename MESH::SH_Vert;
	using SH_Poly = typename MESH::SH_Poly;

	struct Poly {
		std::array<SH_Vert, 3> verts;

		auto hash() const { return Hash<decltype(verts)>()(verts); }
		bool operator==(const Poly& o) const { return verts == o.verts; }
	};

	auto poly_from_p = [](auto&& p) {
		Poly poly;
		poly.verts = {
			p.vert(0),
			p.vert(1),
			p.vert(2),
		};
		std::sort(poly.verts.begin(), poly.verts.end()); // todo: implement salgo sort
		return poly;
	};

	struct Poly_Data {
		int score = 0;
		bool first_done = false;
	};

	auto same_dir = [](auto&& poly, auto&& p) {
		List<SH_Vert> verts;
		for(auto& pv : p.polyVerts()) verts.emplace_back( pv.vert() );

		while(verts[FIRST] != poly.verts[0]) {
			verts.emplace_back( verts[FIRST] );
			verts(FIRST).erase();
		}

		return verts(FIRST).next()() == poly.verts[1];
	};

	Hash_Table<Poly, Poly_Data> polys;

	for(auto& p : mesh.polys()) {
		auto poly = poly_from_p(p);
		auto node = polys.emplace_if_not_found(poly);

		if(same_dir(poly, p)) node-> score++;
		else node-> score--;
	}

	for(auto& p : mesh.polys()) {
		auto poly = poly_from_p(p);
		auto node = polys(poly);

		if(node->score == 0) {
			p.erase();
			continue;
		}

		if(!node->first_done && (node->score > 0) == same_dir(poly, p)) {
			node->first_done = true;
			continue;
		}

		p.erase();
		continue;
	}
}



} // namespace salgo::geom::g3d
