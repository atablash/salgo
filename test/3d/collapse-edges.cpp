#include <salgo/geom/3d/mesh>
#include <salgo/geom/3d/solid>
#include <salgo/geom/3d/cap-holes>
#include <salgo/geom/3d/collapse-edges>

#include <gtest/gtest.h>

#include "common.hpp"

using namespace salgo::geom::geom_3d;




using M = Mesh<double> ::EDGE_LINKS ::VERT_POLY_LINKS ::POLYS_ERASABLE ::VERTS_ERASABLE;




TEST(Fast_collapse_edges, sphere_solid) {
	auto mesh = load_ply<M>("resources/sphere-holes.ply");
	erase_isolated_verts(mesh);
	EXPECT_NE( 0, mesh.verts().domain() );
	EXPECT_TRUE( is_solid(mesh, Check_Solid_Flags::ALLOW_HOLES) );

	fast_compute_edge_links(mesh);
	EXPECT_TRUE( is_solid(mesh, Check_Solid_Flags::ALLOW_HOLES) );

	fast_collapse_edges(mesh, 0.8);
	EXPECT_TRUE( is_solid(mesh, Check_Solid_Flags::ALLOW_HOLES) );

	EXPECT_EQ(8, mesh.verts().count());
	EXPECT_EQ(8, mesh.polys().count());
}



TEST(Fast_collapse_edges, bunny_ply) {
	M mesh = load_ply<Mesh<float>>("resources/bunny-holes.ply");

	erase_isolated_verts(mesh);
	EXPECT_NE( 0, mesh.verts().domain() );

	fast_compute_edge_links(mesh);
	EXPECT_TRUE( is_solid(mesh, Check_Solid_Flags::ALLOW_HOLES) );

	fast_collapse_edges(mesh, 0.02);
	EXPECT_TRUE( is_solid(mesh, Check_Solid_Flags::ALLOW_HOLES) );

	clean_flat_surfaces_on_edges(mesh);
	EXPECT_TRUE( is_solid(mesh, Check_Solid_Flags::ALLOW_HOLES) );
}





TEST(Fast_collapse_edges, bunny_ply_solid) {
	M mesh = load_ply<Mesh<float>>("resources/bunny-holes.ply");
	
	erase_isolated_verts(mesh);
	EXPECT_NE( 0, mesh.verts().domain() );

	fast_compute_edge_links(mesh);
	cap_holes(mesh);
	EXPECT_TRUE( is_solid(mesh) );

	fast_collapse_edges(mesh, 0.01);
	EXPECT_TRUE( is_solid(mesh) );

	clean_flat_surfaces_on_edges(mesh);
	EXPECT_TRUE( is_solid(mesh) );
}







