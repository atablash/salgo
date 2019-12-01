#pragma once

#include "../../hash-table.hpp"

namespace salgo::geom::geom_3d {




template<class MESH>
void move_verts(MESH& mesh, const typename MESH::Vector& d) {
    for(auto& v : mesh.verts()) v.pos() += d;
}


template<class MESH>
void invert_polys(MESH& mesh) {
    for(auto& p : mesh.polys()) {
        auto a = p.vert(1);
        auto b = p.vert(2);

        p.polyVert(1).change_vert(b);
        p.polyVert(2).change_vert(a);
    }
}


template<class MESH>
void append(MESH& mesh, const MESH& other) {
    using H_Vert = typename MESH::H_Vert;

    Hash_Table<H_Vert, H_Vert> v_remap;
    v_remap.reserve( other.verts().count() );

    for(auto& v : other.verts()) {
        auto new_vert = mesh.verts().add( v.pos() );
        if constexpr(MESH::Has_Vert_Data) new_vert.data() = v.data();

        v_remap.emplace( v, new_vert );
    }

    for(auto& p : other.polys()) {
        auto new_poly = mesh.polys().add(
            v_remap[ p.vert(0) ],
            v_remap[ p.vert(1) ],
            v_remap[ p.vert(2) ]
        );

        if constexpr(MESH::Has_Poly_Data) new_poly.data() = p.data();

        if constexpr(MESH::Has_PolyVert_Data) {
            for(int i=0; i<3; ++i) {
                new_poly.poly_vert(i).data() = p.poly_vert(i).data();
            }
        }

        if constexpr(MESH::Has_PolyEdge_Data) {
            for(int i=0; i<3; ++i) {
                new_poly.poly_edge(i).data() = p.poly_edge(i).data();
            }
        }
    }
}



} // namespace salgo
