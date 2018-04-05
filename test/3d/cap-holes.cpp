#include <salgo/3d/mesh.hpp>
#include <salgo/3d/edge-links.hpp>
#include <salgo/3d/vert-poly-links.hpp>
#include <salgo/3d/solid.hpp>
#include <salgo/3d/cap-holes.hpp>
#include <salgo/3d/io.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

using namespace salgo;




using Mesh = Mesh<double>;





TEST(Cap_holes, sphere_holes_ply) {

	auto mesh = load_ply<Mesh>("sphere-holes.ply");
	EXPECT_FALSE(mesh.verts.empty());

	fast_compute_edge_links(mesh);
	compute_vert_poly_links(mesh);

	cap_holes(mesh);

	EXPECT_TRUE( has_valid_edge_links(mesh) );
	EXPECT_TRUE( has_all_edge_links(mesh) );

	EXPECT_TRUE( has_valid_vert_poly_links(mesh) );

	EXPECT_TRUE( is_solid(mesh) );
}




TEST(Cap_holes, bunny_holes_ply) {

	auto mesh = load_ply<Mesh>("bunny-holes.ply");
	EXPECT_FALSE(mesh.verts.empty());

	fast_compute_edge_links(mesh);
	compute_vert_poly_links(mesh);

	auto r = cap_holes(mesh);

	EXPECT_EQ(5, r.num_holes_capped);

	EXPECT_TRUE( has_valid_edge_links(mesh) );
	EXPECT_TRUE( has_all_edge_links(mesh) );

	EXPECT_TRUE( has_valid_vert_poly_links(mesh) );

	EXPECT_TRUE( is_solid(mesh) );
}







