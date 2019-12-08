#pragma once

#include "../../union-find.inl"
#include "../../named-arguments.hpp"

namespace salgo::geom::g3d {


NAMED_ARGUMENT(SUPPORT_HOLES)

template<class MESH, class... ARGS>
void remove_inverted(MESH& mesh, ARGS&&... _args) {
	auto args = Named_Arguments{ std::forward<ARGS>(_args)... };

	using Scalar = typename MESH::Scalar;

	Union_Find ::DATA<Scalar> uf( mesh.verts().domain() );

	for(auto& p : mesh.polys()) {
		uf.merge((int)p.vert(0).handle(), (int)p.vert(1).handle());
		uf.merge((int)p.vert(1).handle(), (int)p.vert(2).handle());
	}

	for(auto& p : mesh.polys()) {
		Eigen::Matrix<Scalar,3,3> mat;
		mat <<
			p.vert(0).pos(),
			p.vert(1).pos(),
			p.vert(2).pos();

		Scalar volume = mat.determinant();

		uf[(int)p.vert(0).handle()] += volume;
	}

	for(auto& p : mesh.polys()) {
		if(uf[(int)p.vert(0).handle()] < 0) {
			p.erase();
		}
	}
}



} // namespace salgo::geom::g3d
