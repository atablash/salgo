#include "resources.hpp"

#include <salgo/geom/3d/io>

using namespace salgo::geom::geom_3d;

Mesh<float> get_bunny_holes() {
	static const auto bunny = load_ply<Mesh<float>>("resources/bunny-holes.ply");
	return bunny;
}

