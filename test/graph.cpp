#include <salgo/graph.hpp>

#include <gtest/gtest.h>

using namespace salgo;





TEST(Graph, construct) {
	Graph g;

	EXPECT_TRUE( g.verts().empty() );
	EXPECT_EQ(0, g.verts().count() );
}


TEST(Graph, construct_verts) {
	Graph g(3);

	EXPECT_FALSE( g.verts().empty() );
	EXPECT_EQ( 3, g.verts().count() );
}


TEST(Graph, add_edge) {
	Graph g(2);
	g.edges().add(0, 1);

	EXPECT_EQ( 1, g.vert(0).outs().count() );
	EXPECT_EQ( 1, g.vert(1).outs().count() );

	// 0 -> 1
	// 1 -> 0
	EXPECT_EQ( 1, g.vert(0).outs(FIRST).vert().handle() );
	EXPECT_EQ( 0, g.vert(1).outs(FIRST).vert().handle() );
}


