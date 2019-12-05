#pragma once

#include <Eigen/Dense>

namespace salgo::linalg {

template<class CONT>
auto fit_line(const CONT& cont) {
    using Vector = std::remove_cv_t<
        std::remove_reference_t<decltype(cont[ANY])>
    >;

    using Scalar = typename Vector::Scalar;

    static constexpr int Dim = Vector::SizeAtCompileTime;
    static_assert(Dim > 0, "dynamic vector sizes not supported (yet)");

    // copy coordinates to  matrix in Eigen format
    auto count = cont.count();
    Eigen::Matrix< Scalar, Eigen::Dynamic, Dim > centers(count, Dim);

    {
        int i=0;
        for(auto& e : cont) centers.row(i++) = e();
    }

    struct Result {
        Vector origin;
        Vector axis;
    };
    Result r;

    r.origin = centers.colwise().mean();
    auto centered = (centers.rowwise() - r.origin.transpose()).eval();
    auto cov = (centered.adjoint() * centered).eval();
    auto eig = Eigen::SelfAdjointEigenSolver<decltype(cov)>(cov);
    r.axis = eig.eigenvectors().col(2).normalized();

    return r;
}

} // namespace salgo::linalg
