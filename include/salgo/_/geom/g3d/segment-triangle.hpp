#pragma once

namespace salgo::geom::g3d {



// real-time collision detection, page 186
// 2-sided
template<class Segment, class Triangle>
auto orient_segment_triangle(const Segment& segment, const Triangle& triangle) {
	using Vector = std::remove_cv_t<
		std::remove_reference_t< decltype(segment.vert(0).pos()) >
	>;

	using Scalar = std::remove_reference_t< decltype( std::declval<Vector>()[0] ) >;

    struct Result {
		Vector point;
		Scalar t01, t02;
		bool cut = false;
    };
    Result result;

	Vector ab = triangle.vert(1).pos() - triangle.vert(0).pos();
	Vector ac = triangle.vert(2).pos() - triangle.vert(0).pos();
	Vector qp = segment.vert(0).pos() - segment.vert(1).pos(); // why qp, not pq?

	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	Vector n = ab.cross(ac);

	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	Scalar d = qp.dot(n);
	if(d <= std::numeric_limits<Scalar>::epsilon() * 1) return result;

	//std::cout << "orient_segment_triangle: d == " << d << std::endl;
	//std::cout << "num epsilons: " << d / std::numeric_limits<Scalar>::epsilon() << std::endl;

	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	Vector ap = segment.vert(0).pos() - triangle.vert(0).pos();
	Scalar t = ap.dot(n);
	if(t < 0) return result;
	if(t > d) return result; // For segment; exclude this code line for a ray test

	// Compute barycentric coordinate components and test if within bounds
	Vector e = qp.cross(ap);
	Scalar v = ac.dot(e);
	if(v < 0 || v > d) return result;

	auto w = -ab.dot(e);
	if(w < 0 || v + w > d) return result;

	// Segment/ray intersects triangle. Perform delayed division and
	// compute the last barycentric coordinate component
	Scalar ood = Scalar(1) / d;
	//t *= ood; // not needed
	v *= ood;
	w *= ood;
	// auto u = (F)1 - v - w;

	result.cut = true;
	result.point = triangle.vert(0).pos() + v*ab + w*ac;
	result.t01 = v;
	result.t02 = w;

	return result;
}


}
