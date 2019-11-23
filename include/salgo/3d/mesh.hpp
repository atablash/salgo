#pragma once


#include "../common.hpp"
#include "../vector-allocator.hpp"
#include "../vector.hpp"
#include "../segment.hpp"
#include "../hash-table.hpp"

#include <Eigen/Dense>

#include <array>

#include "../helper-macros-on"




namespace salgo {
namespace internal {

namespace mesh {



	//
	// add members
	//
	ADD_MEMBER(data);
	ADD_MEMBER(subvert);
	ADD_MEMBER(subvert_link);
	ADD_MEMBER(link);
	ADD_MEMBER(poly_links);
	ADD_MEMBER(edges);





	enum class Erasable {
		NOT_ERASABLE = 0,
		ERASABLE_HOLES,
		ERASABLE_REORDER
	};
	namespace {
		constexpr auto NOT_ERASABLE     = Erasable:: NOT_ERASABLE;
		constexpr auto ERASABLE_HOLES   = Erasable:: ERASABLE_HOLES;
		constexpr auto ERASABLE_REORDER = Erasable:: ERASABLE_REORDER;
	}






	template<
		class _SCALAR,
		class _VERT_DATA,
		class _SUBVERT_DATA,
		class _POLY_DATA,
		class _POLY_VERT_DATA,
		class _POLY_EDGE_DATA,
		Erasable _VERTS_ERASABLE,
		Erasable _POLYS_ERASABLE,
		bool _EDGE_LINKS,
		bool _VERT_POLY_LINKS
	>
	struct Context {

		using Scalar          = _SCALAR;

		using Vert_Data      = _VERT_DATA;
		using Subvert_Data   = _SUBVERT_DATA;
		using Poly_Data      = _POLY_DATA;
		using Poly_Vert_Data = _POLY_VERT_DATA;
		using Poly_Edge_Data = _POLY_EDGE_DATA;

		static constexpr Erasable Verts_Erasable_Mode = _VERTS_ERASABLE;
		static constexpr auto Verts_Erasable = Verts_Erasable_Mode != NOT_ERASABLE;

		static constexpr Erasable Polys_Erasable_Mode = _POLYS_ERASABLE;
		static constexpr auto Polys_Erasable = Polys_Erasable_Mode != NOT_ERASABLE;

		static constexpr bool Has_Edge_Links      = _EDGE_LINKS;
		static constexpr bool Has_Vert_Poly_Links = _VERT_POLY_LINKS;



		static constexpr bool Has_Vert_Data =
			!std::is_same_v<Vert_Data, void>;

		static constexpr bool Has_Subvert_Data =
			!std::is_same_v<Subvert_Data, void>;

		static constexpr bool Has_Poly_Data =
			!std::is_same_v<Poly_Data, void>;

		static constexpr bool Has_Poly_Vert_Data =
			!std::is_same_v<Poly_Vert_Data, void>;

		static constexpr bool Has_Poly_Edge_Data =
			!std::is_same_v<Poly_Edge_Data, void>;









		class Mesh;

		struct Vert;
		struct Poly;


		//
		// handles - forward declarations
		//
		struct H_Poly_Vert;
		struct H_Poly_Edge;



		using Verts = std::conditional_t<Verts_Erasable_Mode == ERASABLE_HOLES,
			salgo::Vector_Allocator<Vert>,
			std::conditional_t< Verts_Erasable_Mode == ERASABLE_REORDER,
				salgo::Vector< Vert >,
				salgo::Vector< Vert > // NOT_ERASABLE
			>
		>;

		using Polys = std::conditional_t<Polys_Erasable_Mode == ERASABLE_HOLES,
			salgo::Vector_Allocator<Poly>,
			std::conditional_t< Polys_Erasable_Mode == ERASABLE_REORDER,
				salgo::Vector< Poly >,
				salgo::Vector< Poly > // NOT_ERASABLE
			>
		>;

		using   H_Vert = typename Verts::Handle;
		using  SH_Vert = typename Verts::Handle_Small;
		using IDX_Vert = typename Verts::Index;

		using   H_Poly = typename Polys::Handle;
		using  SH_Poly = typename Polys::Handle_Small;
		using IDX_Poly = typename Polys::Index;

		struct H_Subvert : Int_Handle_Base<H_Subvert> { FORWARDING_CONSTRUCTOR(H_Subvert, Int_Handle_Base<H_Subvert>) {} };


		struct H_Poly_Vert {
			H_Poly poly = H_Poly();
			int ith = 3; // 0,1,2 - char would be enough

			H_Poly_Vert() = default;
			H_Poly_Vert(H_Poly new_poly, char new_ith) : poly(new_poly), ith(new_ith) {}

			auto next(int i = 1) const { return H_Poly_Vert( poly, (            ith +i) % 3 ); }
			auto prev(int i = 1) const { return H_Poly_Vert( poly, (333'333'333+ith -i) % 3 ); }

			inline auto next_edge() const { return H_Poly_Edge( poly, (  ith   )     ); }
			inline auto prev_edge() const { return H_Poly_Edge( poly, (3+ith -1) % 3 ); }

			bool valid() const { return poly.valid(); }
			void reset() { poly.reset(); }

			bool operator==(const H_Poly_Vert& o) const { return poly == o.poly && ith == o.ith; }
			bool operator!=(const H_Poly_Vert& o) const { return poly != o.poly || ith != o.ith; }

			auto hash() const { return (poly << 2) | ith; }
		};


		struct H_Poly_Edge {
			H_Poly poly = H_Poly();
			int ith = 3; // 0,1,2 - char would be enough

			H_Poly_Edge() = default;
			H_Poly_Edge(H_Poly new_poly, char new_ith) : poly(new_poly), ith(new_ith) {}

			auto next(int i = 1) const { return H_Poly_Edge( poly, (            ith +i) % 3 ); }
			auto prev(int i = 1) const { return H_Poly_Edge( poly, (333'333'333+ith -i) % 3 ); }

			inline auto next_vert() const { return H_Poly_Vert( poly, (ith +1) % 3 ); }
			inline auto prev_vert() const { return H_Poly_Vert( poly, (ith   )     ); }

			bool valid() const { return poly.valid(); }
			void reset() { poly.reset(); }

			bool operator==(const H_Poly_Edge& o) const { return poly == o.poly && ith == o.ith; }
			bool operator!=(const H_Poly_Edge& o) const { return poly != o.poly || ith != o.ith; }

			auto hash() const { return (poly << 2) | ith; }
		};











		using Vert_Poly_Links = salgo::Hash_Table<H_Poly_Vert>;

		struct H_Vert_Poly : Pair_Handle_Base<H_Vert_Poly, Int_Handle<>, typename Vert_Poly_Links::Handle> {
			using BASE = Pair_Handle_Base<H_Vert_Poly, Int_Handle<>, typename Vert_Poly_Links::Handle>;
			FORWARDING_CONSTRUCTOR(H_Vert_Poly, BASE) {}
		};





		struct Verts_Context;
		struct Vert_Polys_Context;
		struct Vert_PolyVerts_Context;

		struct Polys_Context;
		struct Poly_Verts_Context;
		struct Poly_PolyVerts_Context;
		struct Poly_PolyEdges_Context;




		//
		// accessors / iterators
		//
		template<Const_Flag C> class A_Verts;

		template<Const_Flag C> using A_Vert = typename Verts_Context::template Accessor<C>;
		template<Const_Flag C> using I_Vert = typename Verts_Context::template Iterator<C>;

		template<Const_Flag C> class A_Vert_Polys;

		template<Const_Flag C> using A_Vert_Poly = typename Vert_Polys_Context::template Accessor<C>;
		template<Const_Flag C> using I_Vert_Poly = typename Vert_Polys_Context::template Iterator<C>;

		template<Const_Flag C> class A_Vert_PolyVerts;

		template<Const_Flag C> using A_Vert_PolyVert = typename Vert_PolyVerts_Context::template Accessor<C>;
		template<Const_Flag C> using I_Vert_PolyVert = typename Vert_PolyVerts_Context::template Iterator<C>;

		template<Const_Flag C> class A_Polys;

		template<Const_Flag C> using A_Poly = typename Polys_Context::template Accessor<C>;
		template<Const_Flag C> using I_Poly = typename Polys_Context::template Iterator<C>;

		template<Const_Flag C> class A_Poly_Verts;

		template<Const_Flag C> using A_Poly_Vert = typename Poly_Verts_Context::template Accessor<C>;
		template<Const_Flag C> using I_Poly_Vert = typename Poly_Verts_Context::template Iterator<C>;

		template<Const_Flag C> class A_Poly_PolyVerts;

		template<Const_Flag C> using A_Poly_PolyVert = typename Poly_PolyVerts_Context::template Accessor<C>;
		template<Const_Flag C> using I_Poly_PolyVert = typename Poly_PolyVerts_Context::template Iterator<C>;

		template<Const_Flag C> class A_Poly_PolyEdges;

		template<Const_Flag C> using A_Poly_PolyEdge = typename Poly_PolyEdges_Context::template Accessor<C>;
		template<Const_Flag C> using I_Poly_PolyEdge = typename Poly_PolyEdges_Context::template Iterator<C>;








		//
		// raw data types
		//
		struct Vert :
				Add_data<Vert_Data, Has_Vert_Data>,
				Add_poly_links<Vert_Poly_Links, Has_Vert_Poly_Links> {

			Eigen::Matrix<Scalar,3,1> pos = {0,0,0};

			template<class... ARGS>
			Vert(ARGS&&... args) : pos( std::forward<ARGS>(args)... ) {}

			Vert(const Vert&) = default;
			Vert(Vert&&) = default;
		};


		struct Poly_Vert :
				Add_data<Poly_Vert_Data, Has_Poly_Vert_Data>,
				Add_subvert_link<H_Subvert, Has_Subvert_Data> {

			H_Vert vert = H_Vert();
		};


		struct Poly_Edge :
			Add_data<Poly_Edge_Data, Has_Poly_Edge_Data>,
			Add_link<H_Poly_Edge, Has_Edge_Links> {};


		struct Poly :
				Add_data<Poly_Data, Has_Poly_Data>,
				Add_edges<std::array<Poly_Edge,3>, Has_Poly_Edge_Data || Has_Edge_Links> {

			std::array<Poly_Vert,3> verts;

			Poly(H_Vert a, H_Vert b, H_Vert c) {
				verts[0].vert = a;
				verts[1].vert = b;
				verts[2].vert = c;
			}
		};






		static auto& raw(      Mesh& m, H_Vert h) {  return m._vs[h];  }
		static auto& raw(const Mesh& m, H_Vert h) {  return m._vs[h];  }

		static auto& raw(      Mesh& m, H_Poly h) {  return m._ps[h];  }
		static auto& raw(const Mesh& m, H_Poly h) {  return m._ps[h];  }

		static auto& raw(      Mesh& m, H_Poly_Edge h) {  return m._ps[ h.poly ].edges[ h.ith ];  }
		static auto& raw(const Mesh& m, H_Poly_Edge h) {  return m._ps[ h.poly ].edges[ h.ith ];  }





		template<Const_Flag C, class DERIVED>
		class A_Vert_Impl {
		public:
			// access vert directly (useful when currently accessing via poly_verts())
			auto vert()       { return A_Vert<C>    (&_c(), _h()); }
			auto vert() const { return A_Vert<CONST>(&_c(), _h()); }

			auto& mesh()       {  return _c();  }
			auto& mesh() const {  return _c();  }

			auto& pos()       {  return _c()._vs[ _h() ].pos;  }
			auto& pos() const {  return _c()._vs[ _h() ].pos;  }

			auto& data()       {  return _c()._vs[ _h() ].data;  }
			auto& data() const {  return _c()._vs[ _h() ].data;  }

			auto operator->()       { return &data(); }
			auto operator->() const { return &data(); }
		
			auto polys()       { return A_Vert_Polys<C>    ( _c(), _h() ); }
			auto polys() const { return A_Vert_Polys<CONST>( _c(), _h() ); }

			auto vert_polys()       { return A_Vert_PolyVerts<C>    ( _c(), _h() ); }
			auto vert_polys() const { return A_Vert_PolyVerts<CONST>( _c(), _h() ); }


			void erase_with_polys() {
				static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
				static_assert(Verts_Erasable, "erase() called on non-erasable vert");
				static_assert(Has_Vert_Poly_Links, "safe_erase() requires VERT_POLY_LINKS");

				// std::cout << "erase vert " << _h() << std::endl;

				auto& poly_links = _c()._vs[ _h() ].poly_links;

				for(auto& p : poly_links) {
					// std::cout << "also erase poly " << p << std::endl;
					_c()(p).erase_with_edge_links();
				}

				_c()._vs( _h() ).destruct();
			}

			void fast_erase() {
				static_assert(C==MUTAB, "called fast_erase() on CONST A_Vert accessor");
				static_assert(Verts_Erasable, "fast_erase() called on non-erasable vert");

				// std::cout << "fast_erase vert " << _h() << std::endl;

				if constexpr(Has_Vert_Poly_Links) {
					auto& poly_links = _c()._vs[ _h() ].poly_links;
					DCHECK_EQ(poly_links.count(), 0) << "ERROR: fast_erase() called on Vert with adjacent Polys";
				}

				_c()._vs( _h() ).destruct();
			}

		private:
			auto& _c()       { return static_cast<      DERIVED*>(this)->_get_mesh(); }
			auto& _c() const { return static_cast<const DERIVED*>(this)->_get_mesh(); }

			auto& _h()       { return static_cast<      DERIVED*>(this)->_get_vert_handle(); }
			auto& _h() const { return static_cast<const DERIVED*>(this)->_get_vert_handle(); }
		};



		//
		// access/iterate verts
		//
		struct Verts_Context {
			using Container = Mesh;
			using Handle = H_Vert;
			static constexpr auto Has_Data = Has_Vert_Data;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Verts_Context>, public A_Vert_Impl<C, Accessor<C>> {
				using BASE = Accessor_Base<C,Verts_Context>;
				friend A_Vert_Impl<C, Accessor<C>>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
				using Mesh = Context::Mesh;

			private:
				auto& _get_mesh() const { return CONT; }
				auto& _get_vert_handle() const { return HANDLE; }
			}; // Accessor


			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Verts_Context> {
				using BASE = Iterator_Base<C,Verts_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() { HANDLE = CONT._vs( HANDLE ).iterator().next(); }
				void _decrement() { HANDLE = CONT._vs( HANDLE ).iterator().prev(); }

			public:
				bool operator!=(End_Iterator) const { return HANDLE < CONT._vs.domain(); }
			}; // Iterator

		}; // Verts_Context








		template<Const_Flag C, class DERIVED>
		class A_Poly_Impl {
		public:
			auto& mesh()       {  return _c();  }
			auto& mesh() const {  return _c();  }

			auto& data()       {  return raw(_c(), _h()).data;  }
			auto& data() const {  return raw(_c(), _h()).data;  }

			auto operator->()       { return &data(); }
			auto operator->() const { return &data(); }


			auto vert(int ith) {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Vert<C    >( &_c(), _c()._ps[_h()].verts[ith].vert );
			};

			auto vert(int ith) const {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Vert<CONST>( &_c(), _c()._ps[_h()].verts[ith].vert );
			};



			auto poly_vert(int ith) {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_PolyVert<C>( &_c(), H_Poly_Vert(_h(), ith) );
			};

			auto poly_vert(int ith) const {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_PolyVert<CONST>( &_c(), H_Poly_Vert(_h(), ith) );
			};



			auto poly_edge(int ith) {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_PolyEdge<C>( &_c(), H_Poly_Edge(_h(), ith) );
			};

			auto poly_edge(int ith) const {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_PolyEdge<CONST>( &_c(), _h(), ith );
			};



			auto verts()       {  return A_Poly_Verts<C>    ( _c(), _h() );  }
			auto verts() const {  return A_Poly_Verts<CONST>( _c(), _h() );  }

			auto poly_verts()       {  return A_Poly_PolyVerts<C>    (_c(), _h());  }
			auto poly_verts() const {  return A_Poly_PolyVerts<CONST>(_c(), _h());  }

			auto poly_edges()       {  return A_Poly_PolyEdges<C>    (_c(), _h());  }
			auto poly_edges() const {  return A_Poly_PolyEdges<CONST>(_c(), _h());  }


		private:
			struct Erase_Params {
				bool unlink_edge_links = Has_Edge_Links;
				bool remove_isolated_verts = Has_Vert_Poly_Links;
			};

		public:
			void erase(const Erase_Params& par = Erase_Params()) {
				static_assert(Polys_Erasable, "erase() requires POLYS_ERASABLE");
				static_assert(C==MUTAB, "called erase() on CONST A_Poly accessor");

				if(par.unlink_edge_links) {
					DCHECK(Has_Edge_Links) << "unlink_edge_links only works with EDGE_LINKS enabled";
				}

				if(par.remove_isolated_verts) {
					DCHECK(Has_Vert_Poly_Links) << "remove_isolated_verts only works with VERT_POLY_LINKS enabled";
				}

				// std::cout << "erase poly " << _h() << std::endl;

				auto& p = _c()._ps[ _h() ];

				// unlink edge links
				if constexpr(Has_Edge_Links) {
					if(par.unlink_edge_links) {
						for(const auto& pe : p.edges) {
							if(pe.link.valid()) {
								_c()._ps[ pe.link.poly].edges[ pe.link.ith ].link.reset();
							}
						}
					}
				}

				// unlink vert->pv links
				if constexpr(Has_Vert_Poly_Links) {
					for(int ith=0; ith < 3; ++ith) {
						auto& pv = _c()._ps[ _h() ].verts[ith];

						auto& v = _c()._vs[pv.vert];
						DCHECK( v.poly_links( H_Poly_Vert(_h(), ith) ).exists() );
						v.poly_links( H_Poly_Vert(_h(), ith) ).erase();

						if(par.remove_isolated_verts && v.poly_links.empty()) {
							// std::cout << "while erasing poly " << _h() << ": also erasing isolated vert " << pv.vert << std::endl;
							_c()._vs(pv.vert).erase();
						}
					}
				}

				_c()._ps( _h() ).erase();
			}
		
		private:
			auto& _c() const { return static_cast<const DERIVED*>(this)->_get_mesh(); }
			auto& _h() const { return static_cast<const DERIVED*>(this)->_get_poly_handle(); }
		};


		struct Vert_Polys_Context {
			using Container = Mesh;
			using Handle = H_Vert_Poly;
			static constexpr auto Has_Data = Has_Poly_Data;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C, Vert_Polys_Context>, public A_Poly_Impl<C, Accessor<C>> {
				using BASE = Accessor_Base<C, Vert_Polys_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			private:
				auto& _get_mesh() const { return CONT; }
				auto& _get_poly_handle() const { return CONT._vs[ HANDLE.a ].poly_links[ HANDLE.b ]; }
			};

			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C, Vert_Polys_Context> {
				using BASE = Iterator_Base<C, Vert_Polys_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() { HANDLE = CONT._vs( HANDLE ).next(); }
				void _decrement() { HANDLE = CONT._vs( HANDLE ).prev(); }

			public:
				bool operator!=(End_Iterator) const { return HANDLE < CONT._vs.domain(); }
			};
		};




		template<Const_Flag C, class DERIVED>
		class A_PolyVert_Impl {
		public:
			auto& mesh()       {  return _c();  }
			auto& mesh() const {  return _c();  }

			auto poly()       {  return A_Poly<C>    (&_c(), _h().poly);  }
			auto poly() const {  return A_Poly<CONST>(&_c(), _h().poly);  }

			auto vert()       {  return A_Vert<C>    (&_c(), _c()._ps[ _h().poly ].verts[ _h().ith ].vert);  }
			auto vert() const {  return A_Vert<CONST>(&_c(), _c()._ps[ _h().poly ].verts[ _h().ith ].vert);  }

			void change_vert(IDX_Vert new_vert) {
				static_assert(C == MUTAB, "change_vert() called on CONST accessor");

				auto h = _h();

				auto old_vert = _c()._ps[ h.poly ].verts[ h.ith ].vert;

				//std::cout << "poly " << h.poly << ": change vert " << old_vert << "->" << new_vert << std::endl;

				if constexpr(Has_Vert_Poly_Links) {
					//std::cout << "before " << _c()._vs[old_vert].poly_links.count() << std::endl;
					_c()._vs[ old_vert ].poly_links( h ).erase();
					_c()._vs[ new_vert ].poly_links.emplace( h );
					//std::cout << "after " << _c()._vs[old_vert].poly_links.count() << std::endl;
				}

				_c()._ps[ h.poly ].verts[ h.ith ].vert = new_vert;
			}

		private:
			auto& _c() const { return static_cast<const DERIVED*>(this)->_get_mesh(); }
			auto& _h() const { return static_cast<const DERIVED*>(this)->_get_polyvert_handle(); }
		};



		struct Vert_PolyVerts_Context {
			using Container = Mesh;
			using Handle = H_Vert_Poly;
			static constexpr auto Has_Data = Has_Poly_Vert_Data;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C, Vert_PolyVerts_Context>, public A_PolyVert_Impl<C, Accessor<C>> {
				using BASE = Accessor_Base<C, Vert_PolyVerts_Context>;
				friend A_PolyVert_Impl<C, Accessor<C>>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

				auto poly_vert()       { return A_Poly_PolyVert<C>    (&CONT, _get_polyvert_handle()); }
				auto poly_vert() const { return A_Poly_PolyVert<CONST>(&CONT, _get_polyvert_handle()); }

			private:
				auto& _get_mesh() const { return CONT; }
				auto& _get_polyvert_handle() const { return CONT._vs[ HANDLE.a ].poly_links[ HANDLE.b ]; }
			};

			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C, Vert_PolyVerts_Context> {
				using BASE = Iterator_Base<C, Vert_PolyVerts_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() { HANDLE.b = CONT._vs[ HANDLE.a ].poly_links( HANDLE.b ).iterator().next(); }
				void _decrement() { HANDLE.b = CONT._vs[ HANDLE.a ].poly_links( HANDLE.b ).iterator().prev(); }

			public:
				bool operator!=(End_Iterator) const {
					return CONT._vs[ HANDLE.a ].poly_links( HANDLE.b ).iterator() != CONT._vs[ HANDLE.a ].poly_links.end();
				}
			};
		};







		struct Polys_Context {
			using Container = Mesh;
			using Handle = H_Poly;
			static constexpr auto Has_Data = Has_Poly_Data;
			static constexpr auto Comparable = true;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Polys_Context>, public A_Poly_Impl<C,Accessor<C>> {
				using BASE = Accessor_Base<C,Polys_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
				using Mesh = Context::Mesh;

			private:
				auto& _get_mesh() const { return CONT; }
				auto& _get_poly_handle() const { return HANDLE; }
				friend A_Poly_Impl<C,Accessor<C>>;
			}; // Accessor


			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Polys_Context> {
				using BASE = Iterator_Base<C,Polys_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() { HANDLE = CONT._ps( HANDLE ).iterator().next(); }
				void _decrement() { HANDLE = CONT._ps( HANDLE ).iterator().prev(); }

			public:
				bool operator!=(End_Iterator) const { return HANDLE < CONT._ps.domain(); }
			}; // Iterator
		};





		struct Poly_Verts_Context {
			using Container = Mesh;
			using Handle = H_Poly_Vert;
			static constexpr auto Has_Data = Has_Vert_Data;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Poly_Verts_Context>, public A_Vert_Impl<C,Accessor<C>> {
				using BASE = Accessor_Base<C,Poly_Verts_Context>;
				friend A_Vert_Impl<C,Accessor<C>>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
				using Mesh = Context::Mesh;
				

				auto prev()       {  return A_Poly_Vert<C>    (&CONT, HANDLE.prev());  }
				auto prev() const {  return A_Poly_Vert<CONST>(&CONT, HANDLE.prev());  }

				auto next()       {  return A_Poly_Vert<C>    (&CONT, HANDLE.next());  }
				auto next() const {  return A_Poly_Vert<CONST>(&CONT, HANDLE.next());  }


				auto prev_poly_edge()       { return A_Poly_PolyEdge<C>    (&CONT, H_Poly_Edge(HANDLE.poly, (HANDLE.ith+3-1)%3) ); }
				auto prev_poly_edge() const { return A_Poly_PolyEdge<CONST>(&CONT, H_Poly_Edge(HANDLE.poly, (HANDLE.ith+3-1)%3) ); }

				auto next_poly_edge()       { return A_Poly_PolyEdge<C>    (&CONT, H_Poly_Edge(HANDLE.poly, HANDLE.ith) ); }
				auto next_poly_edge() const { return A_Poly_PolyEdge<CONST>(&CONT, H_Poly_Edge(HANDLE.poly, HANDLE.ith) ); }


				auto prev_poly_vert()       { return A_Poly_PolyVert<C>    (&CONT, HANDLE.prev() ); }
				auto prev_poly_vert() const { return A_Poly_PolyVert<CONST>(&CONT, HANDLE.prev() ); }

				auto next_poly_vert()       { return A_Poly_PolyVert<C>    (&CONT, HANDLE.next() ); }
				auto next_poly_vert() const { return A_Poly_PolyVert<CONST>(&CONT, HANDLE.next() ); }


				H_Vert handle() const { return _get_vert_handle(); }
				operator auto() const { return handle(); }

			private:
				auto& _get_mesh() const { return CONT; }
				auto& _get_vert_handle() const { return CONT._ps[ HANDLE.poly ].verts[ HANDLE.ith ].vert; }
			}; // Accessor


			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Poly_Verts_Context> {
				using BASE = Iterator_Base<C,Poly_Verts_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() { ++HANDLE.ith; }
				void _decrement() { --HANDLE.ith; }

			public:
				bool operator!=(End_Iterator) const { return HANDLE.ith < 3; }
			}; // Iterator
		};





		struct Poly_PolyVerts_Context {
			using Container = Mesh;
			using Handle = H_Poly_Vert;
			static constexpr auto Has_Data = Has_Poly_Vert_Data;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Poly_PolyVerts_Context>, public A_PolyVert_Impl<C, Accessor<C>> {
				using BASE = Accessor_Base<C,Poly_PolyVerts_Context>;
				friend A_PolyVert_Impl<C, Accessor<C>>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
				using Mesh = Context::Mesh;

				auto prev()       {  return A_Poly_PolyVert<C>    (&CONT, HANDLE.prev());  }
				auto prev() const {  return A_Poly_PolyVert<CONST>(&CONT, HANDLE.prev());  }

				auto next()       {  return A_Poly_PolyVert<C>    (&CONT, HANDLE.next());  }
				auto next() const {  return A_Poly_PolyVert<CONST>(&CONT, HANDLE.next());  }


				auto prev_vert()       {  return A_Poly_Vert<C>    (&CONT, HANDLE.prev());  }
				auto prev_vert() const {  return A_Poly_Vert<CONST>(&CONT, HANDLE.prev());  }

				auto next_vert()       {  return A_Poly_Vert<C>    (&CONT, HANDLE.next());  }
				auto next_vert() const {  return A_Poly_Vert<CONST>(&CONT, HANDLE.next());  }


				auto prev_poly_edge()       { return A_Poly_PolyEdge<C>    (&CONT, H_Poly_Edge(HANDLE.poly, (HANDLE.ith+3-1)%3) ); }
				auto prev_poly_edge() const { return A_Poly_PolyEdge<CONST>(&CONT, H_Poly_Edge(HANDLE.poly, (HANDLE.ith+3-1)%3) ); }

				auto next_poly_edge()       { return A_Poly_PolyEdge<C>    (&CONT, H_Poly_Edge(HANDLE.poly, HANDLE.ith) ); }
				auto next_poly_edge() const { return A_Poly_PolyEdge<CONST>(&CONT, H_Poly_Edge(HANDLE.poly, HANDLE.ith) ); }

			private:
				auto& _get_mesh() const { return CONT; }
				auto& _get_polyvert_handle() const { return HANDLE; }
			}; // Accessor


			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Poly_PolyVerts_Context> {
				using BASE = Iterator_Base<C,Poly_PolyVerts_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() { ++HANDLE.ith; }
				void _decrement() { --HANDLE.ith; }

			public:
				bool operator!=(End_Iterator) const { return HANDLE.ith < 3; }
			}; // Iterator
		};


		struct Poly_PolyEdges_Context {
			using Container = Mesh;
			using Handle = H_Poly_Edge;
			static constexpr auto Has_Data = Has_Poly_Edge_Data;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Poly_PolyEdges_Context> {
				using BASE = Accessor_Base<C,Poly_PolyEdges_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
				using Mesh = Context::Mesh;

			public:
				auto& mesh()       {  _check_exists(); return CONT;  }
				auto& mesh() const {  _check_exists(); return CONT;  }

				auto poly()       {  _check_exists(); return A_Poly<C>    (&CONT, HANDLE.poly);  }
				auto poly() const {  _check_exists(); return A_Poly<CONST>(&CONT, HANDLE.poly);  }

				auto& data()       {  _check_exists(); return raw(CONT, HANDLE).data;  }
				auto& data() const {  _check_exists(); return raw(CONT, HANDLE).data;  }

				auto operator->()       { _check_exists(); return &data(); }
				auto operator->() const { _check_exists(); return &data(); }

				auto prev()       {  _check_exists(); return A_Poly_PolyEdge<C>    (&CONT, HANDLE.prev());  }
				auto prev() const {  _check_exists(); return A_Poly_PolyEdge<CONST>(&CONT, HANDLE.prev());  }

				auto next()       {  _check_exists(); return A_Poly_PolyEdge<C>    (&CONT, HANDLE.next());  }
				auto next() const {  _check_exists(); return A_Poly_PolyEdge<CONST>(&CONT, HANDLE.next());  }

				auto prev_vert()       { _check_exists(); return A_Poly_Vert<C>    (&CONT, H_Poly_Vert(HANDLE.poly, HANDLE.ith) ); }
				auto prev_vert() const { _check_exists(); return A_Poly_Vert<CONST>(&CONT, H_Poly_Vert(HANDLE.poly, HANDLE.ith) ); }

				auto next_vert()       { _check_exists(); return A_Poly_Vert<C>    (&CONT, H_Poly_Vert(HANDLE.poly, (HANDLE.ith+1)%3) ); }
				auto next_vert() const { _check_exists(); return A_Poly_Vert<CONST>(&CONT, H_Poly_Vert(HANDLE.poly, (HANDLE.ith+1)%3) ); }

				auto prev_poly_vert()       { _check_exists(); return A_Poly_PolyVert<C>    (&CONT, H_Poly_Vert(HANDLE.poly, HANDLE.ith) ); }
				auto prev_poly_vert() const { _check_exists(); return A_Poly_PolyVert<CONST>(&CONT, H_Poly_Vert(HANDLE.poly, HANDLE.ith) ); }

				auto next_poly_vert()       { _check_exists(); return A_Poly_PolyVert<C>    (&CONT, H_Poly_Vert(HANDLE.poly, (HANDLE.ith+1)%3) ); }
				auto next_poly_vert() const { _check_exists(); return A_Poly_PolyVert<CONST>(&CONT, H_Poly_Vert(HANDLE.poly, (HANDLE.ith+1)%3) ); }

				auto segment() const {
					_check_exists();

					return Segment<Scalar, 3>{
						prev_vert().pos(),
						next_vert().pos()
					};
				}

				bool has_link() const {
					_check_exists();

					return raw(CONT, HANDLE).link.valid();
				}

				void link(const Accessor& other) {
					static_assert(C == MUTAB, "link() called on CONST accessor");
					_check_exists();

					auto& this_end  = CONT._ps[HANDLE.poly].edges[HANDLE.ith].link;
					auto& other_end = CONT._ps[other.handle().poly].edges[other.handle().ith].link;

					DCHECK( !this_end.valid() );
					DCHECK( !other_end.valid() );

					this_end = other.handle();
					other_end = HANDLE;

					_check_link();
				}

				void unlink() {
					static_assert(C == MUTAB, "unlink() called on CONST accessor");
					_check_exists();
					DCHECK( has_link() ) << "not satisfied before call to unlink()";
					_check_link();

					auto other = raw(CONT, HANDLE).link;
					raw(CONT, HANDLE).link.reset();
					raw(CONT, other).link.reset();
				}

				void unlink_if_linked() {
					if(has_link()) unlink();
				}

				auto linked_edge() {
					static_assert(Has_Edge_Links, "linked_edge() requires EDGE_LINKS");
					_check_exists();

					auto other = raw(CONT, HANDLE).link;
					// DCHECK( other.valid() ) << "following broken edge link";
					return A_Poly_PolyEdge<C>(&CONT, other);
				}

				auto linked_edge() const {
					_check_exists();

					auto other = raw(CONT, HANDLE).link;
					// DCHECK( other.valid() ) << "following broken edge link";
					return A_Poly_PolyEdge<CONST>(&CONT, other);
				}

				bool exists() const { return HANDLE.valid(); }

			private:
				void _check_link() const {
					auto& this_links_to = CONT._ps[ HANDLE.poly ].edges[ HANDLE.ith ].link;
					DCHECK( this_links_to.valid() );

					auto& other_links_to = CONT._ps[ this_links_to.poly ].edges[ this_links_to.ith ].link;
					DCHECK( other_links_to.valid() );
					DCHECK( other_links_to == HANDLE );
				}

				void _check_exists() const {
					DCHECK(exists()) << "non-existing poly_edge accessed";
				}
			}; // Accessor


			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Poly_PolyEdges_Context> {
				using BASE = Iterator_Base<C,Poly_PolyEdges_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() { ++HANDLE.ith; }
				void _decrement() { --HANDLE.ith; }

			public:
				bool operator!=(End_Iterator) const { return HANDLE.ith < 3; }
			}; // Iterator
		};





		template<Const_Flag C>
		class A_Verts {
		public:
			auto domain() const {  return _mesh._vs.domain();  }

			auto empty() const {  return _mesh._vs.empty();  }
			auto count() const {  return _mesh._vs.count();  }

			void reserve(int num) { _mesh._vs.reserve(num); }

			auto add(const Scalar& x, const Scalar& y, const Scalar& z) {
				auto v = _mesh._vs.add(x, y, z);
				return A_Vert<MUTAB>(&_mesh, v.handle());
			}

			auto begin()       {  return I_Vert<C>    (&_mesh, _mesh._vs.begin());  }
			auto begin() const {  return I_Vert<CONST>(&_mesh, _mesh._vs.begin());  }

			auto end() const {  return typename Verts_Context::End_Iterator(); }

		private:
			A_Verts(Const<Mesh,C>& mesh) : _mesh(mesh) {}
			friend Context;

		private:
			Const<Mesh,C>& _mesh;
		};






		template<Const_Flag C>
		class A_Polys {
		public:
			auto domain() const {  return _mesh._ps.domain();  }

			auto empty() const {  return _mesh._ps.empty();  }
			auto count() const {  return _mesh._ps.count();  }

			void reserve(int num) { _mesh._ps.reserve(num); }

			auto add(IDX_Vert a, IDX_Vert b, IDX_Vert c) {
				auto p = _mesh._ps.add( a, b, c );
				auto acc = A_Poly<MUTAB>(&_mesh, p.handle());

				if constexpr(Has_Vert_Poly_Links) {
					_mesh._vs[ a ].poly_links.emplace( acc.poly_vert(0) );
					_mesh._vs[ b ].poly_links.emplace( acc.poly_vert(1) );
					_mesh._vs[ c ].poly_links.emplace( acc.poly_vert(2) );
				}

				return acc;
			}

			auto begin()       {  return I_Poly<C>    (&_mesh, _mesh._ps.begin());  }
			auto begin() const {  return I_Poly<CONST>(&_mesh, _mesh._ps.begin());  }

			auto end() const {  return typename Polys_Context::End_Iterator();  }

		private:
			A_Polys(Const<Mesh,C>& mesh) : _mesh(mesh) {}
			friend Context;

		private:
			Const<Mesh,C>& _mesh;
		};




		template<Const_Flag C>
		class A_Vert_Polys {
		public:
			auto empty() const { return _mesh._vs[_vert].poly_links.empty(); }
			auto count() const { return _mesh._vs[_vert].poly_links.count(); }

			auto begin()       { return I_Vert_Poly<C>    (&_mesh, H_Vert_Poly(_vert, _mesh._vs[_vert].poly_links.begin())); }
			auto begin() const { return I_Vert_Poly<CONST>(&_mesh, H_Vert_Poly(_vert, _mesh._vs[_vert].poly_links.begin())); }

			auto end()   const { return Vert_Polys_Context::End_Iterator(); }

		private:
			A_Vert_Polys(Const<Mesh,C>& mesh, H_Vert poly) : _mesh(mesh), _vert(poly) {}
			friend Context;

		private:
			Const<Mesh,C>& _mesh;
			const H_Vert _vert;
		};


		template<Const_Flag C>
		class A_Vert_PolyVerts {
		public:
			auto begin()       { return I_Vert_PolyVert<C>    (&_mesh, H_Vert_Poly(_vert, _mesh._vs[_vert].poly_links.begin())); }
			auto begin() const { return I_Vert_PolyVert<CONST>(&_mesh, H_Vert_Poly(_vert, _mesh._vs[_vert].poly_links.begin())); }

			auto end()   const { return typename Vert_PolyVerts_Context::End_Iterator(); }

			auto count() const { return _mesh._vs[_vert].poly_links.count(); }

		private:
			A_Vert_PolyVerts(Const<Mesh,C>& mesh, H_Vert poly) : _mesh(mesh), _vert(poly) {}
			friend Context;

		private:
			Const<Mesh,C>& _mesh;
			const H_Vert _vert;
		};



		template<Const_Flag C>
		class A_Poly_Verts {
		public:
			auto begin()       {  return I_Poly_Vert<C>    (&_mesh, H_Poly_Vert(_poly, 0) );  }
			auto begin() const {  return I_Poly_Vert<CONST>(&_mesh, H_Poly_Vert(_poly, 0) );  }

			auto end() const {  return typename Poly_Verts_Context::End_Iterator();  }

		private:
			A_Poly_Verts(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
			friend Context;

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly _poly;
		};




		template<Const_Flag C>
		class A_Poly_PolyVerts {
		public:
			auto begin()       {  return I_Poly_PolyVert<C>    (&_mesh, H_Poly_Vert(_poly, 0));  }
			auto begin() const {  return I_Poly_PolyVert<CONST>(&_mesh, H_Poly_Vert(_poly, 0));  }

			auto end() const {  return typename Poly_PolyVerts_Context::End_Iterator();  }

		public:
			A_Poly_PolyVerts(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly _poly;
		};





		template<Const_Flag C>
		class A_Poly_PolyEdges {
		public:
			auto begin()       {  return I_Poly_PolyEdge<C>    (&_mesh, H_Poly_Edge(_poly, 0));  }
			auto begin() const {  return I_Poly_PolyEdge<CONST>(&_mesh, H_Poly_Edge(_poly, 0));  }

			auto end() const {  return typename Poly_PolyEdges_Context::End_Iterator();  }

		public:
			A_Poly_PolyEdges(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly _poly;
		};














		//
		//
		//
		class Mesh {
		public:
			using Scalar          = Context::Scalar;

			using Vert_Data      = Context::Vert_Data;
			using Subvert_Data   = Context::Subvert_Data;
			using Poly_Data      = Context::Poly_Data;
			using Poly_Vert_Data = Context::Poly_Vert_Data;
			using Poly_Edge_Data = Context::Poly_Edge_Data;

			static constexpr auto Verts_Erasable_Mode = Context::Verts_Erasable_Mode;
			static constexpr auto Verts_Erasable = Context::Verts_Erasable;
			
			static constexpr auto Polys_Erasable_Mode = Context::Polys_Erasable_Mode;
			static constexpr auto Polys_Erasable = Context::Polys_Erasable;


			using H_Vert      = Context::H_Vert;
			using H_Poly      = Context::H_Poly;
			using H_Poly_Vert = Context::H_Poly_Vert;
			using H_Poly_Edge = Context::H_Poly_Edge;
			using H_Subvert   = Context::H_Subvert;


			static constexpr bool Has_Vert_Data      = Context::Has_Vert_Data;
			static constexpr bool Has_Subvert_Data   = Context::Has_Subvert_Data;
			static constexpr bool Has_Poly_Data      = Context::Has_Poly_Data;
			static constexpr bool Has_Poly_Vert_Data = Context::Has_Poly_Vert_Data;
			static constexpr bool Has_Poly_Edge_Data = Context::Has_Poly_Edge_Data;


			static constexpr bool Has_Edge_Links      = Context::Has_Edge_Links;
			static constexpr bool Has_Vert_Poly_Links = Context::Has_Vert_Poly_Links;

		private:
			friend Context;


		private:
			Verts _vs;
			Polys _ps;



			//
			// accessors from handles
			//
		public:
			auto vert(IDX_Vert handle)       {  return A_Vert<MUTAB>(this, handle);  }
			auto vert(IDX_Vert handle) const {  return A_Vert<CONST>(this, handle);  }

			auto poly(IDX_Poly handle)       {  return A_Poly<MUTAB>(*this, handle);  }
			auto poly(IDX_Poly handle) const {  return A_Poly<CONST>(*this, handle);  }


			auto operator()(H_Vert handle)       {  return A_Vert<MUTAB>(this, handle);  }
			auto operator()(H_Vert handle) const {  return A_Vert<CONST>(this, handle);  }

			auto& operator[](H_Vert handle)       {  return A_Vert<MUTAB>(this, handle).data();  }
			auto& operator[](H_Vert handle) const {  return A_Vert<CONST>(this, handle).data();  }


			auto operator()(H_Poly handle)       {  return A_Poly<MUTAB>(this, handle);  }
			auto operator()(H_Poly handle) const {  return A_Poly<CONST>(this, handle);  }

			auto& operator[](H_Poly handle)       {  return raw(*this, handle).data;  }
			auto& operator[](H_Poly handle) const {  return raw(*this, handle).data;  }


			auto operator()(H_Poly_Vert handle)       {  return A_Poly_PolyVert<MUTAB>(this, handle);  }
			auto operator()(H_Poly_Vert handle) const {  return A_Poly_PolyVert<CONST>(this, handle);  }

			auto& operator[](H_Poly_Vert handle)       {  return A_Poly_PolyVert<MUTAB>(this, handle).data();  }
			auto& operator[](H_Poly_Vert handle) const {  return A_Poly_PolyVert<CONST>(this, handle).data();  }


			auto operator()(H_Poly_Edge handle)       {  return A_Poly_PolyEdge<MUTAB>(this, handle);  }
			auto operator()(H_Poly_Edge handle) const {  return A_Poly_PolyEdge<CONST>(this, handle);  }


			auto& operator[](H_Poly_Edge handle)       {  return raw(*this, handle).data;  }
			auto& operator[](H_Poly_Edge handle) const {  return raw(*this, handle).data;  }



		public:
			auto verts()       {  return A_Verts<MUTAB>(*this);  }
			auto verts() const {  return A_Verts<CONST>(*this);  }

			auto polys()       {  return A_Polys<MUTAB>(*this);  }
			auto polys() const {  return A_Polys<CONST>(*this);  }

		}; // class Mesh










		struct With_Builder : Mesh {

			template<class NEW_VERT_DATA>
			using VERT_DATA =
				typename Context<Scalar, NEW_VERT_DATA, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_SUBVERT_DATA>
			using SUBVERT_DATA =
				typename Context<Scalar, Vert_Data, NEW_SUBVERT_DATA,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_POLY_DATA>
			using POLY_DATA =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					NEW_POLY_DATA, Poly_Vert_Data, Poly_Edge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_POLY_VERT_DATA>
			using POLY_VERT_DATA =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, NEW_POLY_VERT_DATA, Poly_Edge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_POLY_EDGE_DATA>
			using POLY_EDGE_DATA =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, NEW_POLY_EDGE_DATA, Verts_Erasable_Mode, Polys_Erasable_Mode,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;


			using VERTS_ERASABLE =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, ERASABLE_HOLES, Polys_Erasable_Mode,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			using POLYS_ERASABLE =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, Verts_Erasable_Mode, ERASABLE_HOLES,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;


			using VERTS_ERASABLE_REORDER =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, ERASABLE_REORDER, Polys_Erasable_Mode,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			using POLYS_ERASABLE_REORDER =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, Verts_Erasable_Mode, ERASABLE_REORDER,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;


			using EDGE_LINKS =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
					true, Has_Vert_Poly_Links> :: With_Builder;

			using VERT_POLY_LINKS =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, Verts_Erasable_Mode, Polys_Erasable_Mode,
					Has_Edge_Links, true> :: With_Builder;

			using FULL =
				typename Context<Scalar, Vert_Data, Subvert_Data,
					Poly_Data, Poly_Vert_Data, Poly_Edge_Data, ERASABLE_HOLES, ERASABLE_HOLES,
					true, true> :: With_Builder;

		};



	}; // struct Context


} // namespace Mesh
} // namespace internal












template< class T = double >
using Mesh = typename internal::mesh::Context<
	T, // SCALAR
	void,   // VERT_DATA
	void,   // SUBVERT_DATA
	void,   // POLY_DATA
	void,   // POLY_VERT_DATA
	void,   // POLY_EDGE_DATA
	internal::mesh::NOT_ERASABLE,  // VERTS_ERASABLE
	internal::mesh::NOT_ERASABLE,  // POLYS_ERASABLE
	false,  // EDGE_LINKS
	false   // POLY_VERT_LINKS
> :: With_Builder;















} // namespace salgo

#include "../helper-macros-off"



