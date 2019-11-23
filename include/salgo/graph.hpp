#pragma once

#include "common.hpp"
#include "vector-allocator.hpp"
#include "vector.hpp"

#include <array>

#include "helper-macros-on"


namespace salgo {


namespace internal {
namespace graph {



	//
	// add members
	//
	ADD_MEMBER(data);
	ADD_MEMBER(_es);
	ADD_MEMBER(edge);





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
		bool _DIRECTED,
		bool _BACKLINKS, // directed graphs can have 'ins' field
		class _VERT_DATA,
		class _EDGE_DATA,
		class _VERT_EDGE_DATA, // TODO: split into OUT_DATA and IN_DATA
		//class _OUT_DATA,
		//class _IN_DATA,
		Erasable _VERTS_ERASABLE,
		Erasable _EDGES_ERASABLE,
		bool _EDGES_GLOBAL
	>
	struct Context {

		static constexpr bool Directed = _DIRECTED;
		static constexpr bool Backlinks = _BACKLINKS;

		using Vert_Data      = _VERT_DATA;
		using Edge_Data      = _EDGE_DATA;

		using Vert_Edge_Data = _VERT_EDGE_DATA;

		static constexpr Erasable Verts_Erasable = _VERTS_ERASABLE;
		static constexpr Erasable Edges_Erasable = _EDGES_ERASABLE;

		static constexpr bool Edges_Global = _EDGES_GLOBAL;




		static constexpr bool Has_Vert_Data =
				!std::is_same_v<Vert_Data, void>;

		static constexpr bool Has_Edge_Data =
				!std::is_same_v<Edge_Data, void>;

		static constexpr bool Has_Vert_Edge_Data =
				!std::is_same_v<Vert_Edge_Data, void>;






		static constexpr bool Outs_Link_Outs = !Directed;
		static constexpr bool Outs_Link_Ins = Directed && Backlinks;

		static constexpr bool Has_Ins = Directed && Backlinks;




		class Graph;




		struct Vert;
		struct Vert_Edge;
		struct Edge;


		using Verts = std::conditional_t< Verts_Erasable == ERASABLE_HOLES,
		        salgo::Vector_Allocator< Vert >,
				std::conditional_t< Verts_Erasable == ERASABLE_REORDER,
					salgo::Vector< Vert >,
					salgo::Vector< Vert >
				>
		>;

		using Edges = std::conditional_t< Edges_Erasable == ERASABLE_HOLES,
			salgo::Vector_Allocator< Edge >,
				std::conditional_t< Verts_Erasable == ERASABLE_REORDER,
						salgo::Vector< Edge >,
						salgo::Vector< Edge >
				>
		>;

		using Vert_Edges = std::conditional_t< Edges_Erasable == ERASABLE_HOLES,
			salgo::Vector_Allocator< Vert_Edge >,
				std::conditional_t< Verts_Erasable == ERASABLE_REORDER,
						salgo::Vector< Vert_Edge >,
						salgo::Vector< Vert_Edge >
				>
		>;


		using   H_Vert = typename Verts::Handle;
		using  SH_Vert = typename Verts::Handle_Small;
		using IDX_Vert = typename Verts::Index;

		using   H_Edge = typename Edges::Handle;
		using  SH_Edge = typename Edges::Handle_Small;
		using IDX_Edge = typename Edges::Index;

		//using   H_Out  = typename Vert_Edges::Handle;
		//using  SH_Out  = typename Vert_Edges::Handle_Small;
		//using IDX_Out  = typename Vert_Edges::Index;


		struct H_Vert_Edge_Base : Pair_Handle_Base< H_Vert_Edge_Base, H_Vert, typename Vert_Edges::Handle > {
			using BASE = Pair_Handle_Base< H_Vert_Edge_Base, H_Vert, typename Vert_Edges::Handle >;
			FORWARDING_CONSTRUCTOR(H_Vert_Edge_Base, BASE) {}
		};

		struct H_Out : H_Vert_Edge_Base { FORWARDING_CONSTRUCTOR(H_Out, H_Vert_Edge_Base){} };
		struct H_In  : H_Vert_Edge_Base { FORWARDING_CONSTRUCTOR(H_In,  H_Vert_Edge_Base){} };

		template<int oi> using H_Vert_Edge = std::conditional_t<oi == 0, H_Out, H_In>;




		// global edge
		struct Edge : Add_data<Edge_Data, Has_Edge_Data> {
			using BASE = Add_data<Edge_Data, Has_Edge_Data>;

			template<class... ARGS>
			Edge(IDX_Vert, IDX_Vert, ARGS&&... args) : BASE(std::forward<ARGS>(args)...) {}
		};



		struct Vert_Edge :
				Add_edge<SH_Edge, Has_Edge_Data || Edges_Global>,
				Add_data<Vert_Edge_Data, Has_Vert_Edge_Data> {
				
			static constexpr bool Links_Vert_Edge = Outs_Link_Outs || Outs_Link_Ins;
			std::conditional_t<Links_Vert_Edge, H_Vert_Edge_Base, H_Vert> link;

			auto& vert() { if constexpr(Links_Vert_Edge) return link.a; else return link; }
		};


		struct Vert :
				Add_data<Vert_Data, Has_Vert_Data> {

			std::array<Vert_Edges, Has_Ins ? 2 : 1> outs_ins;

			auto& outs()       { return outs_ins[0]; }
			auto& outs() const { return outs_ins[0]; }

			auto& ins()       { static_assert(Has_Ins); return outs_ins[1]; }
			auto& ins() const { static_assert(Has_Ins); return outs_ins[1]; }
		};






		static auto& raw_vs(      Graph& g) {  return g._vs;  }
		static auto& raw_vs(const Graph& g) {  return g._vs;  }

		static auto& raw(      Graph& g, H_Vert h) {  return g._vs[h];  }
		static auto& raw(const Graph& g, H_Vert h) {  return g._vs[h];  }




		struct Verts_Context;
		struct Edges_Context;
		template<int> struct Vert_Edges_Context;






		// Accessors forward declarations
		template<Const_Flag C> using A_Vert = typename Verts_Context::template Accessor<C>;
		template<Const_Flag C> using I_Vert = typename Verts_Context::template Iterator<C>;

		template<Const_Flag C> using A_Edge = typename Edges_Context::template Accessor<C>;
		template<Const_Flag C> using I_Edge = typename Edges_Context::template Iterator<C>;

		template<Const_Flag C, int oi_idx> using A_Vert_Edge = typename Vert_Edges_Context<oi_idx>::template Accessor<C>;
		template<Const_Flag C, int oi_idx> using I_Vert_Edge = typename Vert_Edges_Context<oi_idx>::template Iterator<C>;

		//template<Const_Flag C> using A_In  = typename Ins_Context::template Accessor<C>;
		//template<Const_Flag C> using I_In  = typename Ins_Context::template Iterator<C>;

		template<Const_Flag C> class A_Verts;
		template<Const_Flag C> class A_Edges;
		template<Const_Flag, int> class A_Vert_Edges;






		//
		// access/iterate verts
		//
		struct Verts_Context {
			using Container = Graph;
			using Handle = H_Vert;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Verts_Context> {
				using BASE = Accessor_Base<C,Verts_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				auto& graph()       { return CONT; }
				auto& graph() const { return CONT; }

				auto& data()       { return CONT._vs[ HANDLE ].data; }
				auto& data() const { return CONT._vs[ HANDLE ].data; }

				auto operator->()       { return &data(); }
				auto operator->() const { return &data(); }


				auto outs()       { return A_Vert_Edges<C    ,0>( CONT, HANDLE ); }
				auto outs() const { return A_Vert_Edges<CONST,0>( CONT, HANDLE ); }

				auto ins()       { return A_Vert_Edges<C    ,1>( CONT, HANDLE ); }
				auto ins() const { return A_Vert_Edges<CONST,1>( CONT, HANDLE ); }

				template<class X> auto out(const X& x)       { return outs()(x); }
				template<class X> auto out(const X& x) const { return outs()(x); }

				template<class X> auto in(const X& x)       { return ins()(x); }
				template<class X> auto in(const X& x) const { return ins()(x); }


				void erase() {
					static_assert(Verts_Erasable != NOT_ERASABLE, "called erase() on non-verts-erasable graph");
					static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");

					// remove edges
					if constexpr(Edges_Erasable != NOT_ERASABLE) {
						if constexpr(Outs_Link_Outs) {
							for(auto& outt : CONT._vs[ HANDLE ].outs()) {
								CONT._vs[outt().link.a].outs()(outt().link.b).erase();
							}
						}
						if constexpr(Outs_Link_Ins) {
							for(auto& outt : CONT._vs[ HANDLE ].outs()) {
								CONT._vs[outt().link.a].ins()(outt().link.b).erase();
							}
							for(auto& outt : CONT._vs[ HANDLE ].ins()) {
								CONT._vs[outt().link.a].outs()(outt().link.b).erase();
							}
						}
					}
					else {
						for( auto& outs : CONT._vs[HANDLE].outs_ins ) {
							DCHECK( outs.empty() ) << "removing this vert removes edges, so requires Edges_Erasable";
						}
					}

					CONT._vs( HANDLE ).erase();
				}

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













		//
		// access/iterate edges
		//
		struct Edges_Context {
			using Container = Graph;
			using Handle = H_Edge;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Edges_Context> {
				using BASE = Accessor_Base<C,Edges_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				auto& graph()       {  return CONT;  }
				auto& graph() const {  return CONT;  }

				auto& data()       {  return CONT._es[ HANDLE ].data;  }
				auto& data() const {  return CONT._es[ HANDLE ].data;  }

				auto operator->()       { return &data(); }
				auto operator->() const { return &data(); }


				// TODO:
				//void erase() {
				//	static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
				//	CONT._es.erase( _handle );
				//}

			}; // Accessor


			struct End_Iterator {};


			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Edges_Context> {
				using BASE = Iterator_Base<C,Edges_Context>;

			private:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;
				void _increment() { HANDLE = CONT._es( HANDLE ).iterator().next(); }
				void _decrement() { HANDLE = CONT._es( HANDLE ).iterator().prev(); }

			public:
				bool operator!=(End_Iterator) const { return HANDLE < CONT._es.domain(); }
			}; // Iterator

		}; // Verts_Context














		template<int oi>
		struct Vert_Edges_Context {
			using Container = Graph;
			using Handle = H_Vert_Edge<oi>;


			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Vert_Edges_Context> {
				using BASE = Accessor_Base<C,Vert_Edges_Context>;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				auto& graph()       {  return CONT;  }
				auto& graph() const {  return CONT;  }


				auto& data() {
					if constexpr(Has_Vert_Edge_Data) return _raw().data();
					else DCHECK(false);
					return *this; // never reached
				}

				auto& data() const {
					if constexpr(Has_Vert_Edge_Data) return _raw().data();
					else DCHECK(false);
					return *this; // never reached
				}


				auto vert() { return CONT.vert( _raw().vert() ); }
				auto edge() { return CONT.edge( _raw().edge ); }


				// TODO
				//void erase() {
				//	static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
				//	if constexpr(Outs_Link_Outs) CONT._vs[ _raw().link.a ].outs( _raw().link.b ).erase();
				//	if constexpr(Outs_Link_Ins)  CONT._vs[ _raw().link.a ].ins( _raw().link.b ).erase();
				//	CONT._vs[ HANDLE.a ].outs( HANDLE.b ).erase();
				//}

			private:
				auto& _raw()       { return _raw_outs()[ HANDLE.b ]; }
				auto& _raw() const { return _raw_outs()[ HANDLE.b ]; }

				auto& _raw_outs()       { return CONT._vs[ HANDLE.a ].outs_ins[oi]; }
				auto& _raw_outs() const { return CONT._vs[ HANDLE.a ].outs_ins[oi]; }
			}; // Accessor


			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Vert_Edges_Context> {
				using BASE = Iterator_Base<C,Vert_Edges_Context>;

			public:
				FORWARDING_CONSTRUCTOR( Iterator, BASE ) {}

			private:
				friend BASE;
				void _increment() {	HANDLE.b = _raw_outs()(HANDLE.b).iterator().next(); }
				void _decrement() { HANDLE.b = _raw_outs()(HANDLE.b).iterator().prev(); }

			private:
				auto& _raw_outs()       { return CONT._vs[ HANDLE.a ].outs_ins[oi]; }
				auto& _raw_outs() const { return CONT._vs[ HANDLE.a ].outs_ins[oi]; }

			public:
				bool operator!=(End_Iterator) const { return HANDLE.b != _raw_outs().domain(); }
			}; // Iterator

		}; // Outs_Context













		//
		// plural
		//
		template<Const_Flag C>
		class A_Verts {
		public:
			auto domain() const {  return raw_vs(_graph).domain();  }

			auto empty() const {  return raw_vs(_graph).empty();  }
			auto count() const {  return raw_vs(_graph).count();  }

			void resize(int new_size) {
				if constexpr(Verts_Erasable != NOT_ERASABLE) {
					for(int i=_graph._vs.domain()-1; i>=new_size; --i) {
						if(_graph.vert(i).exists()) _graph.vert(i).erase();
					}
				}
				else {
					DCHECK(new_size > _graph._vs.domain()) << "can't shrink vertices list if not Verts_Erasable";
				}
				_graph._vs.resize(new_size);
			}


			auto begin()       {  return I_Vert<C>    (&_graph, IDX_Vert(0));  }
			auto begin() const {  return I_Vert<CONST>(&_graph, IDX_Vert(0));  }

			auto end() const {  return typename Verts_Context::End_Iterator(); }


		private:
			A_Verts(Const<Graph,C>& graph) : _graph(graph) {}
			friend Graph;

		private:
			Const<Graph,C>& _graph;
		};





		template<Const_Flag C>
		class A_Edges {
		public:
			auto domain() const {  return _graph._es.domain();  }

			auto empty() const {  return _graph._es.empty();  }
			auto count() const {  return _graph._es.count();  }

			template<class... ARGS>
			auto add(IDX_Vert fr, IDX_Vert to, ARGS&&... args) {
				if constexpr(Directed && !Backlinks) {
					auto out = _graph._vs[fr].outs().add();
					out().link = to;
					if constexpr(Has_Edge_Data || Edges_Global) {
						out().edge = _graph._es.add( fr, to, std::forward<ARGS>(args)... );
					}
				}
				else if constexpr(Directed && Backlinks) {
					auto fr_out = _graph._vs[fr].outs().add();
					auto to_in  = _graph._vs[to].ins().add();
					fr_out().link = H_Vert_Edge_Base{ to, to_in };
					to_in().link  = H_Vert_Edge_Base{ fr, fr_out };
					if constexpr(Has_Edge_Data || Edges_Global) {
						auto edge = _graph._es.add( fr, to, std::forward<ARGS>(args)... );
						fr_out().edge = edge;
						to_in().edge = edge;
					}
				}
				else if constexpr(!Directed) {
					auto fr_out = _graph._vs[fr].outs().add();
					auto to_out = _graph._vs[to].outs().add();
					fr_out().link = H_Vert_Edge_Base{ to, to_out };
					to_out().link = H_Vert_Edge_Base{ fr, fr_out };
					if constexpr(Has_Edge_Data || Edges_Global) {
						auto edge = _graph._es.add( fr, to, std::forward<ARGS>(args)... );
						fr_out().edge = edge;
						to_out().edge = edge;
					}
				}
				else {
					DCHECK(false);
				}
			}

			auto begin()       {  return I_Edge<C>    (&_graph, 0);  }
			auto begin() const {  return I_Edge<CONST>(&_graph, 0);  }

			auto end() const {  return Edges_Context::End_Iterator(); }


		public:
			A_Edges(Const<Graph,C>& graph) : _graph(graph) {}

		private:
			Const<Graph,C>& _graph;
		};








		template<Const_Flag C, int oi>
		class A_Vert_Edges {
		public:
			auto domain() const { return _raw().domain(); }

			auto empty() const {  return _raw().empty();  }
			auto count() const {  return _raw().count();  }

			auto operator()(typename Vert_Edges::Index idx)       { return A_Vert_Edge<C,oi>(&_graph, _vert, idx); }
			auto operator()(typename Vert_Edges::Index idx) const { return A_Vert_Edge<C,oi>(&_graph, _vert, idx); }

			auto operator()(First_Tag)       { return A_Vert_Edge<C,oi>(&_graph, H_Vert_Edge<oi>{_vert, _raw()(FIRST)}); }
			auto operator()(First_Tag) const { return A_Vert_Edge<C,oi>(&_graph, H_Vert_Edge<oi>{_vert, _raw()(FIRST)}); }

			auto operator()(Last_Tag)        { return A_Vert_Edge<C,oi>(&_graph, H_Vert_Edge<oi>{_vert, _raw()(LAST)}); }
			auto operator()(Last_Tag)  const { return A_Vert_Edge<C,oi>(&_graph, H_Vert_Edge<oi>{_vert, _raw()(LAST)}); }


			auto begin()       { return I_Vert_Edge<C,oi>(&_graph, H_Vert_Edge_Base{_vert, _raw().begin()}); }
			auto begin() const { return I_Vert_Edge<C,oi>(&_graph, H_Vert_Edge_Base{_vert, _raw().begin()}); }

			auto end() const   { return typename Vert_Edges_Context<oi>::End_Iterator(); }


		private:
			auto& _raw()       { return _graph._vs[_vert].outs_ins[oi]; }
			auto& _raw() const { return _graph._vs[_vert].outs_ins[oi]; }

		public:
			A_Vert_Edges(Const<Graph,C>& graph, H_Vert vert) : _graph(graph), _vert(vert) {}

		private:
			Const<Graph,C>& _graph;
			H_Vert _vert;
		};









		class Graph :
				private Add__es<Edges, Has_Edge_Data> {

			using ADD_ES_BASE = Add__es<Edges, Has_Edge_Data>;

		public:
			using H_Vert = Context::H_Vert;
			using H_Edge = Context::H_Edge;

		private:
			Verts _vs;
			friend Context;

		public:
			Graph() {}
			Graph(int verts_size) : _vs(verts_size) {}

			//
			// accessors from handles
			//
		public:
			auto vert(IDX_Vert handle)       {  return A_Vert<MUTAB>( this, handle );  }
			auto vert(IDX_Vert handle) const {  return A_Vert<CONST>( this, handle );  }

			auto vert(First_Tag)       { return vert( _vs(FIRST) ); }
			auto vert(First_Tag) const { return vert( _vs(FIRST) ); }

			auto vert(Last_Tag)       { return vert( _vs(LAST) ); }
			auto vert(Last_Tag) const { return vert( _vs(LAST) ); }



			auto edge(IDX_Edge handle)       {  return A_Edge<MUTAB>( this, handle );  }
			auto edge(IDX_Edge handle) const {  return A_Edge<CONST>( this, handle );  }

			auto edge(First_Tag)       { return edge( ADD_ES_BASE::_es(FIRST) ); }
			auto edge(First_Tag) const { return edge( ADD_ES_BASE::_es(FIRST) ); }

			auto edge(Last_Tag)       { return edge( ADD_ES_BASE::_es(LAST) ); }
			auto edge(Last_Tag) const { return edge( ADD_ES_BASE::_es(LAST) ); }



			auto operator()(H_Vert handle)       {  return A_Vert<MUTAB>( this, handle );  }
			auto operator()(H_Vert handle) const {  return A_Vert<CONST>( this, handle );  }

			auto& operator[](H_Vert handle)       {  return raw(*this, handle).data;  }
			auto& operator[](H_Vert handle) const {  return raw(*this, handle).data;  }


			auto operator()(H_Edge handle)       {  return A_Edge<MUTAB>( this, handle );  }
			auto operator()(H_Edge handle) const {  return A_Edge<CONST>( this, handle );  }

			auto& operator[](H_Edge handle)       {  return A_Edge<MUTAB>( this, handle ).data();  }
			auto& operator[](H_Edge handle) const {  return A_Edge<CONST>( this, handle ).data();  }


			auto operator()(H_Out handle)       {  return A_Vert_Edge<MUTAB,0>( this, handle );  }
			auto operator()(H_Out handle) const {  return A_Vert_Edge<CONST,0>( this, handle );  }

			auto& operator[](H_Out handle)       {  return A_Vert_Edge<MUTAB,0>( this, handle ).data();  }
			auto& operator[](H_Out handle) const {  return A_Vert_Edge<MUTAB,0>( this, handle ).data();  }


			auto operator()(H_In handle)       {  return A_Vert_Edge<MUTAB,1>( this, handle );  }
			auto operator()(H_In handle) const {  return A_Vert_Edge<CONST,1>( this, handle );  }

			auto& operator[](H_In handle)       {  return A_Vert_Edge<MUTAB,1>( this, handle ).data();  }
			auto& operator[](H_In handle) const {  return A_Vert_Edge<CONST,1>( this, handle ).data();  }


		public:
			auto verts()       {  return A_Verts<MUTAB>( *this );  }
			auto verts() const {  return A_Verts<CONST>( *this );  }

			auto edges()       {  return A_Edges<MUTAB>( *this );  }
			auto edges() const {  return A_Edges<CONST>( *this );  }
		};









		struct With_Builder : Graph {
			FORWARDING_CONSTRUCTOR( With_Builder, Graph ) {}

			using DIRECTED = typename Context<true, Backlinks, Vert_Data,
					Edge_Data, Vert_Edge_Data, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;

			using BACKLINKS = typename Context<Directed, true, Vert_Data,
					Edge_Data, Vert_Edge_Data, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;

			template<class DATA>
			using VERT_DATA = typename Context<Directed, Backlinks, DATA,
					Edge_Data, Vert_Edge_Data, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;

			template<class DATA>
			using EDGE_DATA = typename Context<Directed, Backlinks, Vert_Data,
					DATA, Vert_Edge_Data, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;

			template<class DATA>
			using VERT_EDGE_DATA = typename Context<Directed, Backlinks, Vert_Data,
					Edge_Data, DATA, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;


			using VERTS_ERASABLE = typename Context<Directed, Backlinks, Vert_Data,
					Edge_Data, Vert_Edge_Data, ERASABLE_HOLES,
					Edges_Erasable, Edges_Global> ::With_Builder;

			using EDGES_ERASABLE = typename Context<Directed, Backlinks, Vert_Data,
					Edge_Data, Vert_Edge_Data, Verts_Erasable,
					ERASABLE_HOLES, Edges_Global> ::With_Builder;


			using VERTS_ERASABLE_REORDER = typename Context<Directed, Backlinks, Vert_Data,
					Edge_Data, Vert_Edge_Data, ERASABLE_REORDER,
					Edges_Erasable, Edges_Global> ::With_Builder;

			using EDGES_ERASABLE_REORDER = typename Context<Directed, Backlinks, Vert_Data,
					Edge_Data, Vert_Edge_Data, Verts_Erasable,
					ERASABLE_REORDER, Edges_Global> ::With_Builder;
			
		};

	};

}
}




using Graph = internal::graph::Context<
	false, // directed
	false, // backlinks
	void, // vert data
	void, // edge data
	void, // vert-edge data
	internal::graph::NOT_ERASABLE, // verts erasable
	internal::graph::NOT_ERASABLE, // edges erasable
	false // edges_global
> ::With_Builder;





} // namespace salgo

#include "helper-macros-off"
