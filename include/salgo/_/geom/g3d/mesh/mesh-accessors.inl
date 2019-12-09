#pragma once

#include "../named-arguments.hpp"

#include "../../point-base.hpp"
#include "../../segment-base.hpp"
#include "../../triangle-base.hpp"

#include "../../../helper-macros-on.inc"

#include <Eigen/Dense>

namespace salgo::geom::g3d::_::mesh {

//
// access/iterate verts
//
template<class P>
struct Verts_Context {
	using Container = typename P::Mesh;
	using Handle = typename P::H_Vert;



	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Verts_Context>, public Point_Base<Accessor<C>> {
		using BASE = Accessor_Base<C,Verts_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		using Mesh = typename P::Mesh;

		auto& mesh()       {  return CONT;  }
		auto& mesh() const {  return CONT;  }

		auto& pos()       {  return P::raw(CONT, HANDLE).pos;  }
		auto& pos() const {  return P::raw(CONT, HANDLE).pos;  }

		auto vertPolys()       { return P::template create_vertPolys_accessor<C>    ( CONT, HANDLE ); }
		auto vertPolys() const { return P::template create_vertPolys_accessor<CONST>( CONT, HANDLE ); }

		bool is_constructed() const { return P::raw_vs(CONT)(HANDLE).is_constructed(); }
		bool is_not_constructed() const { return ! is_constructed(); }

		// remove vert, along with its adjacent polys
		void erase_with_polys() {
			static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
			static_assert(P::Verts_Erasable, "erase() called on non-erasable vert");
			static_assert(P::Has_Vert_Poly_Links, "safe_erase() requires VERT_POLY_LINKS");

			// std::cout << "erase vert " << HANDLE << std::endl;

			auto& poly_links = CONT._vs[ HANDLE ].poly_links;

			for(auto& p : poly_links) {
				// std::cout << "also erase poly " << p << std::endl;
				CONT(p).erase_with_edge_links();
			}

			CONT._vs( HANDLE ).destruct();
		}

		// does not check for still existing adjacent polys in Release mode
		template<class... ARGS>
		void fast_erase(ARGS&&... _args) {
			auto args = Named_Arguments{ std::forward<ARGS>(_args)... };

			static_assert(C==MUTAB, "called fast_erase() on CONST A_Vert accessor");
			static_assert(P::Verts_Erasable, "fast_erase() called on non-erasable vert");

			// std::cout << "fast_erase vert " << HANDLE << std::endl;

			if constexpr(P::Has_Vert_Poly_Links) {
				auto& poly_links = P::raw(CONT, HANDLE).poly_links;
				DCHECK_EQ(poly_links.count(), 0) << "ERROR: fast_erase() called on Vert with adjacent Polys";
			}

			if constexpr(args.has(ON_VERT_ERASE)) {
				args(ON_VERT_ERASE)( CONT(HANDLE) );
			}

			P::raw_vs(CONT)( HANDLE ).destruct();
		}
	}; // Accessor


	struct End_Iterator {};

	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Verts_Context> {
		using BASE = Iterator_Base<C,Verts_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() { MUT_HANDLE = P::raw_vs(CONT)( HANDLE ).iterator().next(); }
		void _decrement() { MUT_HANDLE = P::raw_vs(CONT)( HANDLE ).iterator().prev(); }

	public:
		bool operator!=(End_Iterator) const { return HANDLE != P::raw_vs(CONT).domain(); }
	}; // Iterator



	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
}; // Verts_Context













template<class P>
struct VertPolys_Context {
	using Container = typename P::Mesh;
	using Handle = typename P::H_VertPoly;

	template<Const_Flag C>
	class Accessor : public Accessor_Base<C, VertPolys_Context> {
		using BASE = Accessor_Base<C, VertPolys_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

		auto polyVert()       { return P::PolyVerts_Context::template create_accessor<C>    (&CONT, P::raw(CONT, HANDLE.a ).poly_links[ HANDLE.b ]); }
		auto polyVert() const { return P::PolyVerts_Context::template create_accessor<CONST>(&CONT, P::raw(CONT, HANDLE.a ).poly_links[ HANDLE.b ]); }

		auto& mesh()       {  return CONT;  }
		auto& mesh() const {  return CONT;  }

		auto poly()       {  return P::Polys_Context::template create_accessor<C>    (&CONT, P::raw(CONT, HANDLE.a ).poly_links[ HANDLE.b ].poly );  }
		auto poly() const {  return P::Polys_Context::template create_accessor<CONST>(&CONT, P::raw(CONT, HANDLE.a ).poly_links[ HANDLE.b ].poly );  }

		auto vert()       {  return P::A_Vert<C>    (&CONT, HANDLE.vert);  }
		auto vert() const {  return P::A_Vert<CONST>(&CONT, HANDLE.vert);  }
	};

	struct End_Iterator {};

	template<Const_Flag C>
	class Iterator : public Iterator_Base<C, VertPolys_Context> {
		using BASE = Iterator_Base<C, VertPolys_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() { MUT_HANDLE.b = P::raw(CONT, HANDLE.a ).poly_links( HANDLE.b ).iterator().next(); }
		void _decrement() { MUT_HANDLE.b = P::raw(CONT, HANDLE.a ).poly_links( HANDLE.b ).iterator().prev(); }

	public:
		bool operator!=(End_Iterator) const {
			return P::raw(CONT, HANDLE.a ).poly_links( HANDLE.b ).iterator() != P::raw(CONT, HANDLE.a ).poly_links.end();
		}
	};



	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
};





template<class P, Const_Flag C>
class Polys_Accessor : public Accessor_Base<C,Polys_Context<P>>, public Triangle_Base<Polys_Accessor<P,C>> {
	using BASE = Accessor_Base<C,Polys_Context<P>>;

public:
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
	using Mesh = typename P::Mesh;
	using Scalar = typename P::Scalar;

	auto& mesh()       {  return CONT;  }
	auto& mesh() const {  return CONT;  }


	auto vert(int ith) {
		DCHECK_GE(ith, 0) << "out of bounds";
		DCHECK_LT(ith, 3) << "out of bounds";
		return P::Verts_Context::template create_accessor<C    >( &CONT, P::raw(CONT, HANDLE).verts[ith].vert );
	};

	auto vert(int ith) const {
		DCHECK_GE(ith, 0) << "out of bounds";
		DCHECK_LT(ith, 3) << "out of bounds";
		return P::Verts_Context::template create_accessor<CONST>( &CONT, P::raw(CONT, HANDLE).verts[ith].vert );
	};



	auto polyVert(int ith) {
		DCHECK_GE(ith, 0) << "out of bounds";
		DCHECK_LT(ith, 3) << "out of bounds";
		return P::PolyVerts_Context::template create_accessor<C>( &CONT, H_PolyVert<P>(HANDLE, ith) );
	};

	auto polyVert(int ith) const {
		DCHECK_GE(ith, 0) << "out of bounds";
		DCHECK_LT(ith, 3) << "out of bounds";
		return P::PolyVerts_Context::template create_accessor<CONST>( &CONT, H_PolyVert<P>(HANDLE, ith) );
	};



	auto polyEdge(int ith) {
		DCHECK_GE(ith, 0) << "out of bounds";
		DCHECK_LT(ith, 3) << "out of bounds";
		return P::PolyEdges_Context::template create_accessor<C>( &CONT, H_PolyEdge<P>(HANDLE, ith) );
	};

	auto polyEdge(int ith) const {
		DCHECK_GE(ith, 0) << "out of bounds";
		DCHECK_LT(ith, 3) << "out of bounds";
		return P::PolyEdges_Context::template create_accessor<CONST>( &CONT, H_PolyEdge<P>(HANDLE, ith) );
	};


	auto polyVerts()       {  return P::template create_polyVerts_accessor<C>    (CONT, HANDLE);  }
	auto polyVerts() const {  return P::template create_polyVerts_accessor<CONST>(CONT, HANDLE);  }

	auto polyEdges()       {  return P::template create_polyEdges_accessor<C>    (CONT, HANDLE);  }
	auto polyEdges() const {  return P::template create_polyEdges_accessor<CONST>(CONT, HANDLE);  }


	auto aabb() const {
		auto a = vert(0).pos().array();
		auto b = vert(1).pos().array();
		auto c = vert(2).pos().array();

		return Eigen::AlignedBox<Scalar,3> {
			a.min(b).min(c),
			a.max(b).max(c)
		};
	}

	template<class... ARGS>
	void erase(ARGS&&... _args) {
		auto args = Named_Arguments{ std::forward<ARGS>(_args)... };

		auto unlink_edge_links = args(UNLINK_EDGE_LINKS, P::Has_Edge_Links);
		auto erase_isolated_verts = args(ERASE_ISOLATED_VERTS, P::Has_Vert_Poly_Links);

		static_assert(P::Polys_Erasable, "erase() requires POLYS_ERASABLE");
		static_assert(C==MUTAB, "called erase() on CONST A_Poly accessor");

		if constexpr( ! P::Has_Edge_Links) {
			DCHECK(!unlink_edge_links) << "UNLINK_EDGE_LINKS only works with EDGE_LINKS enabled";
		}

		if constexpr( ! P::Has_Vert_Poly_Links) {
			DCHECK(!erase_isolated_verts) << "ERASE_ISOLATED_VERTS only works with VERT_POLY_LINKS enabled";
		}

		auto& p = P::raw( CONT, HANDLE );

		if constexpr(args.has(ON_POLY_ERASE)) {
			args(ON_POLY_ERASE)(*this);
		}

		// unlink edge links
		if constexpr(P::Has_Edge_Links) {
			if(unlink_edge_links) {
				for(const auto& pe : p.edges) {
					if(pe.link.valid()) {
						P::raw(CONT, pe.link).link.reset();
					}
				}
			}
		}

		// unlink vert->pv links
		if constexpr(P::Has_Vert_Poly_Links) {
			for(int ith=0; ith < 3; ++ith) {
				auto& pv = P::raw(CONT, HANDLE).verts[ith];

				auto& v = P::raw(CONT, pv.vert);
				DCHECK( v.poly_links( H_PolyVert<P>(HANDLE, ith) ).found() );
				v.poly_links( H_PolyVert<P>(HANDLE, ith) ).erase();

				if(erase_isolated_verts && v.poly_links.is_empty()) {
					// std::cout << "while erasing poly " << HANDLE << ": also erasing isolated vert " << pv.vert << std::endl;
					if constexpr(args.has(ON_VERT_ERASE)) {
						args(ON_VERT_ERASE)( CONT(pv.vert) );
					}
					P::raw_vs(CONT)(pv.vert).erase();
				}
			}
		}

		// std::cout << "erase poly " << this->handle() << std::endl;

		P::raw_ps(CONT)( HANDLE ).erase();
	}
}; // Polys_Accessor


template<class P, salgo::Const_Flag C>
::std::ostream& operator<<(::std::ostream& os, const Polys_Accessor<P,C>& acc) {
	return os << "{poly " << acc.handle() << ", verts " << acc.vert(0).handle() << " " << acc.vert(1).handle() << " " << acc.vert(2).handle() << "}";
}




template<class P>
struct Polys_Context : private P {
	using Container = typename P::Mesh;
	using Handle = typename P::H_Poly;
	static constexpr auto Comparable = true;

	using typename P::H_PolyVert;
	using typename P::H_PolyEdge;

	template<Const_Flag C>
	using Accessor = mesh::Polys_Accessor<P,C>;


	struct End_Iterator {};

	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Polys_Context> {
		using BASE = Iterator_Base<C,Polys_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() { MUT_HANDLE = P::raw_ps(CONT)( HANDLE ).iterator().next(); }
		void _decrement() { MUT_HANDLE = P::raw_ps(CONT)( HANDLE ).iterator().prev(); }

	public:
		bool operator!=(End_Iterator) const { return HANDLE != P::raw_ps(CONT).domain(); }
	}; // Iterator




	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
};








template<class P>
struct PolyVerts_Context : private P {
	using Container = typename P::Mesh;
	using Handle = typename P::H_PolyVert;

	using typename P::H_PolyEdge;

	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,PolyVerts_Context> {
		using BASE = Accessor_Base<C,PolyVerts_Context>;

	public:
		using BASE::BASE;
		using Mesh = typename P::Mesh;

		auto& mesh()       {  return CONT;  }
		auto& mesh() const {  return CONT;  }

		auto prev()       {  return P::PolyVerts_Context::template create_accessor<C>    (&CONT, HANDLE.prev());  }
		auto prev() const {  return P::PolyVerts_Context::template create_accessor<CONST>(&CONT, HANDLE.prev());  }

		auto next()       {  return P::PolyVerts_Context::template create_accessor<C>    (&CONT, HANDLE.next());  }
		auto next() const {  return P::PolyVerts_Context::template create_accessor<CONST>(&CONT, HANDLE.next());  }


		auto prev_polyEdge()       { return P::PolyEdges_Context::template create_accessor<C>    (&CONT, H_PolyEdge(HANDLE.poly, (HANDLE.ith+2)%3) ); }
		auto prev_polyEdge() const { return P::PolyEdges_Context::template create_accessor<CONST>(&CONT, H_PolyEdge(HANDLE.poly, (HANDLE.ith+2)%3) ); }

		auto next_polyEdge()       { return P::PolyEdges_Context::template create_accessor<C>    (&CONT, H_PolyEdge(HANDLE.poly, HANDLE.ith) ); }
		auto next_polyEdge() const { return P::PolyEdges_Context::template create_accessor<CONST>(&CONT, H_PolyEdge(HANDLE.poly, HANDLE.ith) ); }

		auto opposite_polyEdge()       { return P::PolyEdges_Context::template create_accessor<C>    (&CONT, H_PolyEdge(HANDLE.poly, (HANDLE.ith+1)%3) ); }
		auto opposite_polyEdge() const { return P::PolyEdges_Context::template create_accessor<CONST>(&CONT, H_PolyEdge(HANDLE.poly, (HANDLE.ith+1)%3) ); }

		auto prev_vert()       { return prev().vert(); }
		auto prev_vert() const { return prev().vert(); }

		auto next_vert()       { return next().vert(); }
		auto next_vert() const { return next().vert(); }


		auto vertPoly()       { return P::VertPolys_Context::template create_accessor<C>    (&CONT, HANDLE); }
		auto vertPoly() const { return P::VertPolys_Context::template create_accessor<CONST>(&CONT, HANDLE); }

		auto poly()       {  return P::Polys_Context::template create_accessor<C>    (&CONT, HANDLE.poly);  }
		auto poly() const {  return P::Polys_Context::template create_accessor<CONST>(&CONT, HANDLE.poly);  }

		auto vert()       {  return P::Verts_Context::template create_accessor<C>    (&CONT, P::raw(CONT, HANDLE.poly).verts[ HANDLE.ith ].vert);  }
		auto vert() const {  return P::Verts_Context::template create_accessor<CONST>(&CONT, P::raw(CONT, HANDLE.poly).verts[ HANDLE.ith ].vert);  }

		void change_vert(typename P::IDX_Vert new_vert) {
			static_assert(C == MUTAB, "change_vert() called on CONST accessor");

			auto h = HANDLE;

			auto old_vert = P::raw(CONT, h).vert;

			//std::cout << "poly " << h.poly << ": change vert " << old_vert << "->" << new_vert << std::endl;

			if constexpr(P::Has_Vert_Poly_Links) {
				//std::cout << "before " << CONT._vs[old_vert].poly_links.count() << std::endl;
				P::raw(CONT, old_vert ).poly_links( h ).erase();
				P::raw(CONT, new_vert ).poly_links.emplace( h );
				//std::cout << "after " << CONT._vs[old_vert].poly_links.count() << std::endl;
			}

			P::raw(CONT, h.poly ).verts[ h.ith ].vert = new_vert;
		}
	}; // Accessor


	struct End_Iterator {};

	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,PolyVerts_Context> {
		using BASE = Iterator_Base<C,PolyVerts_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() { ++MUT_HANDLE.ith; }
		void _decrement() { --MUT_HANDLE.ith; }

	public:
		bool operator!=(End_Iterator) const { return HANDLE.ith != 3; }
	}; // Iterator



	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
};





template<class P, Const_Flag C>
class PolyEdge_Accessor : public Accessor_Base<C,PolyEdges_Context<P>>, public Segment_Base<PolyEdge_Accessor<P,C>>, private P {
	using BASE = Accessor_Base<C,PolyEdges_Context<P>>;

	using typename P::H_PolyVert;

public:
	using BASE::BASE;
	using Mesh = typename P::Mesh;

public:
	auto& mesh()       {  _check(); return CONT;  }
	auto& mesh() const {  _check(); return CONT;  }

	auto poly()       {  _check(); return P::Polys_Context::template create_accessor<C>    (&CONT, HANDLE.poly);  }
	auto poly() const {  _check(); return P::Polys_Context::template create_accessor<CONST>(&CONT, HANDLE.poly);  }

	auto& data()       {  _check(); return raw(CONT, HANDLE).data;  }
	auto& data() const {  _check(); return raw(CONT, HANDLE).data;  }

	auto operator->()       { _check(); return &data(); }
	auto operator->() const { _check(); return &data(); }

	auto prev()       {  _check(); return PolyEdge_Accessor<P,C>    (&CONT, HANDLE.prev());  }
	auto prev() const {  _check(); return PolyEdge_Accessor<P,CONST>(&CONT, HANDLE.prev());  }

	auto next()       {  _check(); return PolyEdge_Accessor<P,C>    (&CONT, HANDLE.next());  }
	auto next() const {  _check(); return PolyEdge_Accessor<P,CONST>(&CONT, HANDLE.next());  }


	auto prev_polyVert()       { _check(); return P::PolyVerts_Context::template create_accessor<C>    (&CONT, H_PolyVert(HANDLE.poly, HANDLE.ith) ); }
	auto prev_polyVert() const { _check(); return P::PolyVerts_Context::template create_accessor<CONST>(&CONT, H_PolyVert(HANDLE.poly, HANDLE.ith) ); }

	auto next_polyVert()       { _check(); return P::PolyVerts_Context::template create_accessor<C>    (&CONT, H_PolyVert(HANDLE.poly, (HANDLE.ith+1)%3) ); }
	auto next_polyVert() const { _check(); return P::PolyVerts_Context::template create_accessor<CONST>(&CONT, H_PolyVert(HANDLE.poly, (HANDLE.ith+1)%3) ); }

	auto opposite_polyVert()       { _check(); return P::PolyVerts_Context::template create_accessor<C>    (&CONT, H_PolyVert(HANDLE.poly, (HANDLE.ith+2)%3) ); }
	auto opposite_polyVert() const { _check(); return P::PolyVerts_Context::template create_accessor<CONST>(&CONT, H_PolyVert(HANDLE.poly, (HANDLE.ith+2)%3) ); }


	auto prev_vert()       { _check(); return prev_polyVert().vert(); }
	auto prev_vert() const { _check(); return prev_polyVert().vert(); }

	auto next_vert()       { _check(); return next_polyVert().vert(); }
	auto next_vert() const { _check(); return next_polyVert().vert(); }

	auto opposite_vert()       { _check(); return opposite_polyVert().vert(); }
	auto opposite_vert() const { _check(); return opposite_polyVert().vert(); }


	// segment interface
	auto vert(int ith)       { _check(); DCHECK_LE(0, ith); DCHECK_LT(ith, 2); return ith == 0 ? prev_polyVert().vert() : next_polyVert().vert(); }
	auto vert(int ith) const { _check(); DCHECK_LE(0, ith); DCHECK_LT(ith, 2); return ith == 0 ? prev_polyVert().vert() : next_polyVert().vert(); }


	bool is_linked() const {
		_check();
		static_assert(P::Has_Edge_Links, "is_linked() called on Mesh without EDGE_LINKS enabled");
		return P::raw(CONT, HANDLE).link.valid();
	}

	bool is_not_linked() const {
		return ! is_linked();
	}

	void link(const PolyEdge_Accessor& other) {
		static_assert(C == MUTAB, "link() called on CONST accessor");
		_check();

		auto& this_end  = P::raw(CONT, HANDLE.poly).edges[HANDLE.ith].link;
		auto& other_end = P::raw(CONT, other.handle().poly).edges[other.handle().ith].link;

		DCHECK( !this_end.valid() );
		DCHECK( !other_end.valid() );

		this_end = other.handle();
		other_end = HANDLE;

		_check_link();
	}

	void unlink() {
		static_assert(C == MUTAB, "unlink() called on CONST accessor");
		_check();
		DCHECK( is_linked() ) << "not satisfied before call to unlink()";
		_check_link();

		auto other = P::raw(CONT, HANDLE).link;
		P::raw(CONT, HANDLE).link.reset();
		P::raw(CONT, other).link.reset();
	}

	void unlink_if_linked() {
		if(is_linked()) unlink();
	}

	auto linked_polyEdge() {
		static_assert(P::Has_Edge_Links, "linked_polyEdge() requires EDGE_LINKS");
		_check();

		auto other = P::raw(CONT, HANDLE).link;
		// DCHECK( other.valid() ) << "following broken edge link";
		return P::PolyEdges_Context::template create_accessor<C>(&CONT, other);
	}

	auto linked_polyEdge() const {
		_check();

		auto other = raw(CONT, HANDLE).link;
		// DCHECK( other.valid() ) << "following broken edge link";
		return P::PolyEdges_Context::template create_accessor<CONST>(&CONT, other);
	}

	auto get_aabb() const {
		_check();

		auto a = prev_vert().pos().array();
		auto b = next_vert().pos().array();

		return Eigen::AlignedBox<typename P::Scalar,3>{
			a.min(b),
			a.max(b)
		};
	}

private:
	void _check_link() const {
		auto& this_links_to = P::raw(CONT,HANDLE).link;
		DCHECK( this_links_to.valid() );

		auto& other_links_to = P::raw( CONT, this_links_to.poly ).edges[ this_links_to.ith ].link;
		DCHECK( other_links_to.valid() );
		DCHECK( other_links_to == HANDLE );
	}

	void _check() const {
		DCHECK(BASE::valid()) << "invalid polyEdge accessed";
	}
}; // Accessor


template<class P>
struct PolyEdges_Context : private P {
	using Container = typename P::Mesh;
	using Handle = typename P::H_PolyEdge;

	template<Const_Flag C>
	using Accessor = PolyEdge_Accessor<P,C>;

	using typename P::H_PolyVert;

	struct End_Iterator {};

	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,PolyEdges_Context> {
		using BASE = Iterator_Base<C,PolyEdges_Context>;

	public:
		using BASE::BASE;

	private:
		friend BASE;

		void _increment() { ++ MUT_HANDLE.ith; }
		void _decrement() { -- MUT_HANDLE.ith; }

	public:
		bool operator!=(End_Iterator) const { return HANDLE.ith != 3; }
	}; // Iterator



	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
};


template<class P, salgo::Const_Flag C>
::std::ostream& operator<<(::std::ostream& os, const PolyEdge_Accessor<P,C>& acc) {
	return os << "{polyEdge " << acc.handle() << ", verts " << acc.vert(0).handle() << " " << acc.vert(1).handle() << "}";
}








template<class P, Const_Flag C>
class A_Verts : private P {
public:
	auto domain() const {  return P::raw_vs(_mesh).domain();  }

	auto  is_empty() const {  return P::raw_vs(_mesh).is_empty();  }
	auto not_empty() const {  return !is_empty();  }

	auto count() const {  return P::raw_vs(_mesh).count();  }

	void reserve(int num) { P::raw_vs(_mesh).reserve(num); }

	// auto add(const Scalar& x, const Scalar& y, const Scalar& z) {
	// 	auto v = _mesh._vs.add(x, y, z);
	// 	return A_Vert<MUTAB>(&_mesh, v.handle());
	// }

	template<class... ARGS, class SFINAE = decltype(typename P::Vert(std::declval<ARGS>()...))>
	auto add(ARGS&&... args) {
		auto v = P::raw_vs(_mesh).add( std::forward<ARGS>(args)... );
		return P::Verts_Context::template create_accessor<MUTAB>(&_mesh, v.handle());
	}

	//
	// copy-construct new vertex (copies position and vert_data)
	// disabled for now - isn't obvious what assigning vertex to vertex do
	// better be more explicit in client code
	//
	// template<class AV>
	// auto add(const AV& other) {
	// 	auto v = _mesh._vs.add( other.pos() );
	// 	if constexpr(Has_Vert_Data) {
	// 		if constexpr(AV::Has_Data) {
	// 			v.data() = other.data();
	// 		}
	// 	}
	// 	return A_Vert<MUTAB>(&_mesh, v.handle());
	// }

	auto begin()       {  return P::Verts_Context::template create_iterator<C>    (&_mesh, P::raw_vs(_mesh).begin());  }
	auto begin() const {  return P::Verts_Context::template create_iterator<CONST>(&_mesh, P::raw_vs(_mesh).begin());  }

	auto end() const {  return typename P::Verts_Context::End_Iterator(); }

private:
	A_Verts(Const<typename P::Mesh,C>& mesh) : _mesh(mesh) {}
	friend P;

private:
	Const<typename P::Mesh,C>& _mesh;
};






template<class P, Const_Flag C>
class A_Polys : private P {
	using typename P::IDX_Vert;

public:
	auto domain() const {  return P::raw_ps(_mesh).domain();  }

	auto  is_empty() const {  return P::raw_ps(_mesh).is_empty();  }
	auto not_empty() const {  return !is_empty();  }

	auto count() const {  return P::raw_ps(_mesh).count();  }

	void reserve(int num) { P::raw_ps(_mesh).reserve(num); }

	auto add(IDX_Vert a, IDX_Vert b, IDX_Vert c) {
		// DCHECK_NE(a,b);
		// DCHECK_NE(b,c);
		// DCHECK_NE(c,a);

		auto p = P::raw_ps(_mesh).add( a, b, c );
		auto acc = P::Polys_Context::template create_accessor<MUTAB>(&_mesh, p.handle());

		if constexpr(P::Has_Vert_Poly_Links) {
			P::raw(_mesh, a).poly_links.emplace( acc.polyVert(0) );
			P::raw(_mesh, b).poly_links.emplace( acc.polyVert(1) );
			P::raw(_mesh, c).poly_links.emplace( acc.polyVert(2) );
		}

		return acc;
	}

	auto begin()       {  return P::Polys_Context::template create_iterator<C>    (&_mesh, P::raw_ps(_mesh).begin());  }
	auto begin() const {  return P::Polys_Context::template create_iterator<CONST>(&_mesh, P::raw_ps(_mesh).begin());  }

	auto end() const {  return typename P::Polys_Context::End_Iterator();  }

private:
	A_Polys(Const<typename P::Mesh,C>& mesh) : _mesh(mesh) {}
	friend P;

private:
	Const<typename P::Mesh,C>& _mesh;
};





template<class P, Const_Flag C>
class A_VertPolys : private P {
	using typename P::Mesh;
	using typename P::H_Vert;
	using typename P::H_VertPoly;

public:
	auto begin()       { return P::VertPolys_Context::template create_iterator<C>    (&_mesh, H_VertPoly(_vert, P::raw(_mesh, _vert).poly_links.begin())); }
	auto begin() const { return P::VertPolys_Context::template create_iterator<CONST>(&_mesh, H_VertPoly(_vert, P::raw(_mesh, _vert).poly_links.begin())); }

	auto end()   const { return typename P::VertPolys_Context::End_Iterator(); }

	auto count() const { return P::raw(_mesh, _vert).poly_links.count(); }

	bool is_empty() const { return P::raw(_mesh, _vert).poly_links.is_empty(); }
	bool not_empty() const { return !is_empty(); }

private:
	A_VertPolys(Const<Mesh,C>& mesh, H_Vert poly) : _mesh(mesh), _vert(poly) {}
	friend P;

private:
	Const<typename P::Mesh,C>& _mesh;
	const H_Vert _vert;
};






template<class P, Const_Flag C>
class A_PolyVerts : private P {
	using typename P::Mesh;
	using typename P::H_Poly;
	using typename P::H_PolyVert;

public:
	auto begin()       {  return P::PolyVerts_Context::template create_iterator<C>    (&_mesh, H_PolyVert(_poly, 0));  }
	auto begin() const {  return P::PolyVerts_Context::template create_iterator<CONST>(&_mesh, H_PolyVert(_poly, 0));  }

	auto end() const {  return typename P::PolyVerts_Context::End_Iterator();  }

public:
	A_PolyVerts(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
	friend P;

private:
	Const<Mesh,C>& _mesh;
	const H_Poly _poly;
};





template<class P, Const_Flag C>
class A_PolyEdges : private P {
	using typename P::Mesh;
	using typename P::H_Poly;
	using typename P::H_PolyEdge;

public:
	auto begin()       {  return P::PolyEdges_Context::template create_iterator<C>    (&_mesh, H_PolyEdge(_poly, 0));  }
	auto begin() const {  return P::PolyEdges_Context::template create_iterator<CONST>(&_mesh, H_PolyEdge(_poly, 0));  }

	auto end() const {  return typename P::PolyEdges_Context::End_Iterator();  }

public:
	A_PolyEdges(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
	friend P;

private:
	Const<Mesh,C>& _mesh;
	const H_Poly _poly;
};





} // namespace salgo::geom::g3d::_::mesh






#include "../../../helper-macros-off.inc"
