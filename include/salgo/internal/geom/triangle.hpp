#pragma once

namespace salgo::geom {

template<class SCALAR, int DIM>
class Triangle {
    using Vector = Eigen::Matrix<SCALAR, DIM, 1>;
    std::array<Vector,3> _verts;

public:
    template<class... ARGS>
    Triangle(ARGS&&... args) : _verts{ std::forward<ARGS>(args)... } {}

public:
    auto& vert(int ith)       { return _verts[ith]; }
    auto& vert(int ith) const { return _verts[ith]; }
};


} // namespace salgo::geom
