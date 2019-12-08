#include <salgo/graph/graph>

#include <gtest/gtest.h>

using namespace salgo;
using namespace salgo::graph;





TEST(Graph, construct) {
	Graph g;

	EXPECT_TRUE( g.verts().is_empty() );
	EXPECT_EQ(0, g.verts().count() );
}


TEST(Graph, construct_verts) {
	Graph g(3);

	EXPECT_TRUE( g.verts().not_empty() );
	EXPECT_EQ( 3, g.verts().count() );
}


TEST(Graph, add_edge) {
	Graph g(2);

	g.edges().add(0, 1);

	EXPECT_EQ( 1, g.vert(0).outs().count() );
	EXPECT_EQ( 1, g.vert(1).outs().count() );

	// 0 -> 1
	// 1 -> 0
	EXPECT_EQ( 1, g.vert(0).out(FIRST).vert().handle() );
	EXPECT_EQ( 0, g.vert(1).out(FIRST).vert().handle() );
}


TEST(Graph, directed_add_edge) {
	Graph ::DIRECTED g(2);
	g.edges().add(0, 1);

	EXPECT_EQ( 1, g.vert(0).outs().count() );
	EXPECT_EQ( 0, g.vert(1).outs().count() );

	// 0 -> 1
	EXPECT_EQ( 1, g.vert(0).out(FIRST).vert().handle() );
}


TEST(Graph, directed_backlinks_add_edge) {
	Graph ::DIRECTED ::BACKLINKS g(2);
	g.edges().add(0, 1);

	EXPECT_EQ( 1, g.vert(0).outs().count() );
	EXPECT_EQ( 0, g.vert(1).outs().count() );

	EXPECT_EQ( 0, g.vert(0).ins().count() );
	EXPECT_EQ( 1, g.vert(1).ins().count() );

	// 0 -> 1
	EXPECT_EQ( 1, g.vert(0).out(FIRST).vert().handle() );
	EXPECT_EQ( 0, g.vert(1).in(FIRST).vert().handle() );
}


TEST(Graph, vert_data) {
	Graph ::VERT_DATA<int> g(2);

	g.vert(0).data() = 12;
	g.vert(1).data() = 23;

	EXPECT_EQ( 12, g.vert(0).data() );
	EXPECT_EQ( 23, g.vert(1).data() );
}



TEST(Graph, edge_data) {
	Graph ::EDGE_DATA<int> g(2);
	g.edges().add(0, 1, 123);

	EXPECT_EQ(123, g.vert(FIRST).out(FIRST).edge().data());
}


TEST(Graph, iterate_verts) {
	Graph ::VERT_DATA<int> g(2);
	g.vert(0).data() = 12;
	g.vert(1).data() = 23;

	std::vector<int> r;
	for(auto& e : g.verts()) {
		r.emplace_back( e.data() );
	}

	EXPECT_EQ(std::vector({12,23}), r);
}


TEST(Graph, verts_erase) {
	Graph ::VERTS_ERASABLE g(3);
	g.edges().add(1, 2);

	g.vert(0).erase();

	EXPECT_EQ(1, g.vert(1).outs().count());
	EXPECT_EQ(2, g.vert(1).out(FIRST).vert().handle());

	EXPECT_EQ(1, g.vert(2).outs().count());
	EXPECT_EQ(1, g.vert(2).out(FIRST).vert().handle());
}


TEST(Graph, verts_erase_star) {
	Graph ::VERTS_ERASABLE ::EDGES_ERASABLE g(5);

	g.edges().add(0,1);
	g.edges().add(0,2);
	g.edges().add(3,0);
	g.edges().add(4,0);

	g.vert(1).erase();
	g.vert(3).erase();

	EXPECT_EQ(2, g.vert(0).outs().count());

	std::multiset<int> r;
	for(auto& e : g.vert(0).outs()) {
		r.emplace( e.vert().handle() );
	}
	EXPECT_EQ(std::multiset<int>({2,4}), r);
}
