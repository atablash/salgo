#pragma once

namespace salgo::geom::geom_3d {




// ignores case when `point`'s projection is outside `triangle`
template<class POINT, class TRI>
auto orient_point_triangle(const POINT& point, const TRI& triangle) {
	using Vector = std::remove_const_t<
		std::remove_reference_t< decltype(point.pos()) >
	>;

    using Scalar = std::remove_reference_t< decltype( std::declval<Vector>()[0] ) >;

    struct Result {
        Scalar dist_sqr = std::numeric_limits<Scalar>::max();
        Scalar dist     = std::numeric_limits<Scalar>::max();
    };
    Result result;

	Vector BA = triangle.vert(1).pos() - triangle.vert(0).pos();
	Vector CA = triangle.vert(2).pos() - triangle.vert(0).pos();
	Vector normal = BA.cross(CA);

	Vector AP = triangle.vert(0).pos() - point.pos();
	Vector BP = triangle.vert(1).pos() - point.pos();

	Scalar pab = AP.cross(BP).dot(normal);
	if (pab < 0) return result;

	Vector CP = triangle.vert(2).pos() - point.pos();

	Scalar pbc = BP.cross(CP).dot(normal);
	if (pbc < 0) return result;

	Scalar pca = CP.cross(AP).dot(normal);
	if (pca < 0) return result;

	// optim todo: use lagrange identity: Real-Time Collision Detection, page 140

	Vector R = (
        triangle.vert(0).pos() * pbc +
        triangle.vert(1).pos() * pca +
        triangle.vert(2).pos() * pab) / (pab + pbc + pca);

	result.dist_sqr = (point.pos() - R).squaredNorm();
	result.dist = sqrt(result.dist_sqr);
	return result;
}


}
