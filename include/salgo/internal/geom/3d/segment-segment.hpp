#pragma once

namespace salgo {





// todo: make this return lines-distance when segments are parallel too
template<class SEG>
auto orient_segments(const SEG& a, const SEG& b) {
    using Vector = std::remove_reference_t< decltype(a.vert(0)) >;
    using Scalar = std::remove_reference_t< decltype(a.vert(1)[0]) >;

    struct Result {
        Scalar lines_dist_sqr = std::numeric_limits<Scalar>::max();
        Vector point;
    };
    Result result;

	Vector BA3 = a.vert(1) - a.vert(0);
	Vector DC3 = b.vert(1) - b.vert(0);

	auto m = Eigen::Matrix<Scalar, 2, 3>();
    m << BA3.transpose(), DC3.transpose();

	auto A = (m * a.vert(0)).eval();
	auto B = (m * a.vert(1)).eval();
	auto C = (m * b.vert(0)).eval();
	auto D = (m * b.vert(1)).eval();
	auto BA = (B - A).eval();
	auto DC = (D - C).eval();
	auto CA = (C - A).eval();
	auto BC = (B - C).eval();
	auto DA = (D - A).eval();

    auto det = [](auto& v0, auto& v1){ return v0[0]*v1[1] - v0[1]*v1[0]; };

	Scalar den = det(BA,DC);
	if(den == 0) return result;

	Scalar alpha = det(CA,DA) / den;
	if(alpha < 0 || alpha > 1) return result;

	Scalar beta = det(CA,BC) / den;
	if(beta < 0 || beta > 1) return result;

	Vector p0 = a.vert(0) + BA3 * alpha;
	Vector p1 = b.vert(0) + DC3 * beta;

	result.lines_dist_sqr = (p1-p0).squaredNorm();
	result.point   = (p0+p1) * Scalar(0.5);
	return result;
}



}
