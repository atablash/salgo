#pragma once

#include "../../common.hpp"

#include "edge-links.hpp"
#include "vert-poly-links.hpp"
#include "mesh-utils.hpp"





namespace salgo::geom::geom_3d {





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








enum class Check_Solid_Flags {
	NONE = 0,
	ALLOW_HOLES = 0x0001
};

ENABLE_BITWISE_OPERATORS(Check_Solid_Flags);

namespace {
	static constexpr auto ALLOW_HOLES = Check_Solid_Flags::ALLOW_HOLES;
}






template<class MESH>
auto check_solid(const MESH& mesh, Check_Solid_Flags flags = Check_Solid_Flags::NONE) {

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
		if(!bool(flags & ALLOW_HOLES) && !has_all_edge_links(mesh)) {
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

	(void)flags; // suppress unused warning

	r.is_solid = true;
	return r;
}



template <class MESH>
auto is_solid(const MESH& mesh, Check_Solid_Flags flags = Check_Solid_Flags::NONE) {
	auto r = check_solid(mesh, flags);
	// std::cout << "check_solid_result " << (int)r.failure << std::endl;
	return r.is_solid;
}





} // namespace salgo

