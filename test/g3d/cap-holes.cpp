#include <salgo/geom/g3d/mesh>
#include <salgo/geom/g3d/solid>
#include <salgo/geom/g3d/cap-holes>
#include <salgo/geom/g3d/io>

#include <gtest/gtest.h>

#include "common.hpp"

using namespace salgo::geom::g3d;




using MyMesh = Mesh<double> ::EDGE_LINKS;





TEST(Cap_holes, sphere_holes_ply) {

	auto mesh = load_ply<MyMesh>("resources/sphere-holes.ply");
	EXPECT_FALSE(mesh.verts().empty());

	fast_compute_edge_links(mesh);
	//compute_vert_poly_links(mesh);

	cap_holes(mesh);

	EXPECT_TRUE( has_valid_edge_links(mesh) );
	EXPECT_TRUE( has_all_edge_links(mesh) );

	//EXPECT_TRUE( has_valid_vert_poly_links(mesh) );

	EXPECT_TRUE( is_solid(mesh) );
}




TEST(Cap_holes, bunny_holes_ply) {

	auto mesh = load_ply<MyMesh>("resources/bunny-holes.ply");
	EXPECT_FALSE(mesh.verts().empty());

	fast_compute_edge_links(mesh);
	//compute_vert_poly_links(mesh);

	auto r = cap_holes(mesh);

	EXPECT_EQ(5, r.num_holes_capped);

	EXPECT_TRUE( has_valid_edge_links(mesh) );
	EXPECT_TRUE( has_all_edge_links(mesh) );

	//EXPECT_TRUE( has_valid_vert_poly_links(mesh) );

	EXPECT_TRUE( is_solid(mesh) );
}







