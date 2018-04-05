#include "common.hpp"

#include <salgo/3d/mesh.hpp>
#include <salgo/3d/compute-normals.hpp>

#include <gtest/gtest.h>



using namespace salgo;



struct Vert_Props_normal {
	Eigen::Matrix<double,3,1> normal;
};


using M = Mesh<double> :: VERT_PROPS<Vert_Props_normal>;


TEST(Fast_compute_vert_normals, cube) {

	auto mesh = get_cube_mesh<M>();

	fast_compute_vert_normals(mesh);

	for(int x=0; x<2; ++x) {
		for(int y=0; y<2; ++y) {
			for(int z=0; z<2; ++z) {
				int idx = x*4 + y*2 + z;

				if(x == 0) EXPECT_LT(mesh.vert(idx).props().normal[0], -0.1);
				else EXPECT_GT(mesh.vert(idx).props().normal[0], 0.1);

				if(y == 0) EXPECT_LT(mesh.vert(idx).props().normal[1], -0.1);
				else EXPECT_GT(mesh.vert(idx).props().normal[1], 0.1);

				if(z == 0) EXPECT_LT(mesh.vert(idx).props().normal[2], -0.1);
				else EXPECT_GT(mesh.vert(idx).props().normal[2], 0.1);
			}
		}
	}
}



TEST(Compute_vert_normals, cube) {
	
	auto mesh = get_cube_mesh<M>();

	compute_vert_normals(mesh);

	auto s = 1.0 / sqrt(3);

	for(int x=0; x<2; ++x) {
		for(int y=0; y<2; ++y) {
			for(int z=0; z<2; ++z) {
				int idx = x*4 + y*2 + z;

				if(x == 0) EXPECT_DOUBLE_EQ(-s, mesh.vert(idx).props().normal[0]);
				else EXPECT_DOUBLE_EQ(s, mesh.vert(idx).props().normal[0]);

				if(y == 0) EXPECT_DOUBLE_EQ(-s, mesh.vert(idx).props().normal[1]);
				else EXPECT_DOUBLE_EQ(s, mesh.vert(idx).props().normal[1]);

				if(z == 0) EXPECT_DOUBLE_EQ(-s, mesh.vert(idx).props().normal[2]);
				else EXPECT_DOUBLE_EQ(s, mesh.vert(idx).props().normal[2]);
			}
		}
	}
}






TEST(Compute_vert_normals, cube_external) {
	
	const auto mesh = get_cube_mesh<M>();

	std::vector<Eigen::Matrix<double,3,1>> normals( mesh.verts_domain() );

	compute_vert_normals(mesh, [&normals](int i) -> auto& { return normals[i]; });

	auto s = 1.0 / sqrt(3);

	for(int x=0; x<2; ++x) {
		for(int y=0; y<2; ++y) {
			for(int z=0; z<2; ++z) {
				int idx = x*4 + y*2 + z;

				if(x == 0) EXPECT_DOUBLE_EQ(-s, normals[idx][0]);
				else EXPECT_DOUBLE_EQ(s, normals[idx][0]);

				if(y == 0) EXPECT_DOUBLE_EQ(-s, normals[idx][1]);
				else EXPECT_DOUBLE_EQ(s, normals[idx][1]);

				if(z == 0) EXPECT_DOUBLE_EQ(-s, normals[idx][2]);
				else EXPECT_DOUBLE_EQ(s, normals[idx][2]);
			}
		}
	}
}





