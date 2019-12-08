#pragma once

#include "edge-links.inl"
#include "vert-poly-links.hpp"
#include "mesh-utils.hpp"

#include "../../named-arguments.hpp"

#include <iostream> // todo: disable



namespace salgo::geom::g3d {





template<class MESH>
bool has_degenerate_polys(const MESH& mesh) {
	for(auto& p : mesh.polys()) {
		for(auto& v : p.polyVerts()) {
			if(v.vert() == v.next().vert()) return true;
		}
	}
	return false;
}






struct Check_Solid_Result {

	enum class Failure {
		SUCCESS = 0,
		DEGENERATE_POLYS,
		INVALID_EDGE_LINKS,
		INVALID_VERT_POLY_LINKS,
		ISOLATED_VERTS
	};

	bool is_solid = false;
	Failure failure = Failure::SUCCESS;
};








NAMED_ARGUMENT(ALLOW_HOLES)


template<class MESH, class... ARGS>
auto check_solid(const MESH& mesh, ARGS&&... _args) {
	auto args = Named_Arguments{ std::forward<ARGS>(_args)... };

	const auto allow_holes = args(ALLOW_HOLES, false);

	Check_Solid_Result r;

	if(has_degenerate_polys(mesh)) {
		r.failure = Check_Solid_Result::Failure::DEGENERATE_POLYS;
		return r;
	}

	if constexpr(MESH::Has_Edge_Links) {
		if(!has_valid_edge_links(mesh)) {
			r.failure = Check_Solid_Result::Failure::INVALID_EDGE_LINKS;
			return r;
		}
		if(!allow_holes && !has_all_edge_links(mesh)) {
			r.failure = Check_Solid_Result::Failure::INVALID_EDGE_LINKS;
			return r;
		}
	}

	if constexpr(MESH::Has_Vert_Poly_Links) {
		if(!has_valid_vert_poly_links(mesh)) {
			r.failure = Check_Solid_Result::Failure::INVALID_VERT_POLY_LINKS;
			return r;
		}
	}

	if(has_isolated_verts(mesh)) {
		r.failure = Check_Solid_Result::Failure::ISOLATED_VERTS;
		return r;
	}

	r.is_solid = true;
	return r;
}



template <class MESH, class... ARGS>
auto is_solid(const MESH& mesh, ARGS&&... args) {
	auto r = check_solid(mesh, std::forward<ARGS>(args)...);
	// std::cout << "check_solid_result " << (int)r.failure << std::endl;
	return r.is_solid;
}





} // namespace salgo

