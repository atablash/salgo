#pragma once

#include "../../union-find.inl"
#include "../../named-arguments.hpp"

namespace salgo::geom::g3d {


NAMED_ARGUMENT(CAVE_MODE)
NAMED_ARGUMENT(SUPPORT_HOLES) // todo: not implemented (required some ray-casting)

template<class MESH, class... ARGS>
void remove_inverted(MESH& mesh, ARGS&&... _args) {
	static_assert(MESH::Has_Edge_Links);

	using Scalar = typename MESH::Scalar;

	auto args = Named_Arguments{ std::forward<ARGS>(_args)... };
	auto cave_mode = args(CAVE_MODE, false);

	fast_compute_edge_links(mesh);

	Union_Find ::DATA<Scalar> uf( mesh.polys().domain() );

	for(auto& p : mesh.polys()) {
		for(auto& pe : p.polyEdges()) {
			if(pe.is_not_linked()) continue;

			uf.merge((int)p.handle(), (int)pe.linked_polyEdge().poly().handle());
			uf.merge((int)p.handle(), (int)pe.linked_polyEdge().poly().handle());
		}
	}

	for(auto& p : mesh.polys()) {
		Eigen::Matrix<Scalar,3,3> mat;
		mat <<
			p.vert(0).pos(),
			p.vert(1).pos(),
			p.vert(2).pos();

		Scalar volume = mat.determinant();

		uf[(int)p.handle()] += volume;
	}

	for(auto& p : mesh.polys()) {
		if(uf[(int)p.handle()] * (cave_mode ? -1 : 1) < 0) {
			p.erase();
		}
	}
}



} // namespace salgo::geom::g3d
