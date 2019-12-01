#pragma once

namespace salgo {




// ignores case when `point`'s projection is outside `triangle`
template<class TRI, class Vector>
auto orient_point_triangle(const Vector& point, const TRI& triangle) {
    using Scalar = std::remove_reference_t< decltype(point[0]) >;

    struct Result {
        Scalar dist_sqr = std::numeric_limits<F>::max();
    };
    Result result;

	Vector BA = triangle.vert(1) - triangle.vert(0);
	Vector CA = triangle.vert(2) - triangle.vert(0);
	Vector normal = BA.cross(CA);

	Vector AP = triangle.vert(0) - point;
	Vector BP = triangle.vert(1) - point;

	Scalar pab = normal * AP.cross(BP);
	if (pab < 0) return result;

	Vector CP = triangle.vert(2) - point;

	Scalar pbc = normal * BP.cross(CP);
	if (pbc < 0) return result;

	Scalar pca = normal * CP.cross(AP);
	if (pca < 0) return result;

	// optim todo: use lagrange identity: Real-Time Collision Detection, page 140

	Scalar R = (
        triangle.vert(0) * pbc +
        triangle.vert(1) * pca +
        triangle.vert(2) * pab) / (pab + pbc + pca);

	result.dist_sqr = (point - R).squaredNorm();
	return result;
}


}
