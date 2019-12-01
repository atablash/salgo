#pragma once

namespace salgo::geom::geom_3d::internal::mesh {


ADD_MEMBER(data);
ADD_MEMBER(subvert);
ADD_MEMBER(subvert_link);
ADD_MEMBER(link);
ADD_MEMBER(poly_links);
ADD_MEMBER(edges);


template<class P>
struct H_PolyVert {
	using H_Poly = typename P::H_Poly;

	H_Poly poly = H_Poly();
	int ith = 3; // 0,1,2 - char would be enough

	H_PolyVert() = default;
	H_PolyVert(H_Poly new_poly, char new_ith) : poly(new_poly), ith(new_ith) {}

	auto next(int i = 1) const { return H_PolyVert( poly, (            ith +i) % 3 ); }
	auto prev(int i = 1) const { return H_PolyVert( poly, (333'333'333+ith -i) % 3 ); }

	inline auto next_edge() const { return H_PolyEdge( poly, (  ith   )     ); }
	inline auto prev_edge() const { return H_PolyEdge( poly, (3+ith -1) % 3 ); }

	bool valid() const { return poly.valid(); }
	void reset() { poly.reset(); }

	bool operator==(const H_PolyVert& o) const { return poly == o.poly && ith == o.ith; }
	bool operator!=(const H_PolyVert& o) const { return poly != o.poly || ith != o.ith; }

	auto hash() const { return (poly << 2) | ith; }
};




template<class P>
struct H_PolyEdge {
	using H_Poly = typename P::H_Poly;

	H_Poly poly = H_Poly();
	int ith = 3; // 0,1,2 - char would be enough

	H_PolyEdge() = default;
	H_PolyEdge(H_Poly new_poly, char new_ith) : poly(new_poly), ith(new_ith) {}

	auto next(int i = 1) const { return H_PolyEdge( poly, (            ith +i) % 3 ); }
	auto prev(int i = 1) const { return H_PolyEdge( poly, (333'333'333+ith -i) % 3 ); }

	inline auto next_vert() const { return H_PolyVert( poly, (ith +1) % 3 ); }
	inline auto prev_vert() const { return H_PolyVert( poly, (ith   )     ); }

	bool valid() const { return poly.valid(); }
	void reset() { poly.reset(); }

	bool operator==(const H_PolyEdge& o) const { return poly == o.poly && ith == o.ith; }
	bool operator!=(const H_PolyEdge& o) const { return poly != o.poly || ith != o.ith; }

	auto hash() const { return (poly << 2) | ith; }
};




template<class P>
struct H_VertPoly : Pair_Handle_Base<H_VertPoly<P>, Int_Handle<>, typename P::Vert_Poly_Links::Handle> {
	using BASE = Pair_Handle_Base<H_VertPoly<P>, Int_Handle<>, typename P::Vert_Poly_Links::Handle>;
	using BASE::BASE;
};








//
// raw data types
//
template<class P>
struct Vert :
		Add_data<typename P::Vert_Data, P::Has_Vert_Data>,
		Add_poly_links<typename P::Vert_Poly_Links, P::Has_Vert_Poly_Links> {

	typename P::Vector pos = {0,0,0};

	template<class... ARGS>
	Vert(ARGS&&... args) : pos( std::forward<ARGS>(args)... ) {}

	Vert(const Vert&) = default;
	Vert(Vert&&) = default;
};


template<class P>
struct PolyVert :
		Add_data<typename P::PolyVert_Data, P::Has_PolyVert_Data>/*,
		Add_subvert_link<typename P::H_Subvert, P::Has_Subvert_Data>*/ {

	using H_Vert = typename P::H_Vert;

	H_Vert vert = H_Vert();
};


template<class P>
struct PolyEdge :
	Add_data<typename P::PolyEdge_Data, P::Has_PolyEdge_Data>,
	Add_link<typename P::H_PolyEdge, P::Has_Edge_Links> {};


template<class P>
struct Poly :
		Add_data<typename P::Poly_Data, P::Has_Poly_Data>,
		Add_edges<std::array<PolyEdge<P>,3>, P::Has_PolyEdge_Data || P::Has_Edge_Links> {

	std::array<PolyVert<P>,3> verts;

	using H_Vert = typename P::H_Vert;

	Poly(H_Vert a, H_Vert b, H_Vert c) {
		verts[0].vert = a;
		verts[1].vert = b;
		verts[2].vert = c;
	}
};



} // namespace salgo::geom::geom_3d::internal::mesh


namespace salgo {
	template<class P>
	auto& operator<<(::std::ostream& os, const salgo::geom::geom_3d::internal::mesh::H_PolyEdge<P>& pe) {
		return os << "{poly:" << pe.poly << ", edge:" << pe.ith << "}";
	}
}
