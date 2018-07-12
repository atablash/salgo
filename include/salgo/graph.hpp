#pragma once

#include "common.hpp"
#include "vector-allocator.hpp"
#include "vector.hpp"


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
		NON_ERASABLE = 0,
		ERASABLE_HOLES,
		ERASABLE_REORDER
	};
	namespace {
		constexpr auto NON_ERASABLE     = Erasable:: NON_ERASABLE;
		constexpr auto ERASABLE_HOLES   = Erasable:: ERASABLE_HOLES;
		constexpr auto ERASABLE_REORDER = Erasable:: ERASABLE_REORDER;
	}





	template<
		bool _DIRECTED,
		bool _BACKLINKS, // directed graphs can have 'ins' field
		class _VERT_PROPS,
		//class _OUT_PROPS,
		//class _IN_PROPS,
		class _EDGE_PROPS,
		Erasable _VERTS_ERASABLE,
		Erasable _EDGES_ERASABLE,
		bool _EDGES_GLOBAL
	>
	struct Context {

		static constexpr bool Directed = _DIRECTED;
		static constexpr bool Backlinks = _BACKLINKS;

		using Vert_Data      = _VERT_PROPS;
		using Edge_Data      = _EDGE_PROPS;

		static constexpr Erasable Verts_Erasable = _VERTS_ERASABLE;
		static constexpr Erasable Edges_Erasable = _EDGES_ERASABLE;

		static constexpr bool Edges_Global = _EDGES_GLOBAL;




		static constexpr bool Has_Vert_Data =
			!std::is_same_v<Vert_Data, void>;

		static constexpr bool Has_Edge_Data =
			!std::is_same_v<Edge_Data, void>;






		static constexpr bool Outs_Link_Outs = !Directed;
		static constexpr bool Outs_Link_Ins = Directed && Backlinks;

		static constexpr bool Has_Ins = Directed && Backlinks;



		#define GRAPH_FRIENDS \
			friend A_Vert<MUTAB>; \
			friend A_Vert<CONST>; \
			friend I_Vert<MUTAB>; \
			friend I_Vert<CONST>; \
			friend A_Edge<MUTAB>; \
			friend A_Edge<CONST>; \
			friend I_Edge<MUTAB>; \
			friend I_Edge<CONST>; \
			friend A_Out<MUTAB,0>; \
			friend A_Out<CONST,0>; \
			friend A_Out<MUTAB,1>; \
			friend A_Out<CONST,1>; \
			friend I_Out<MUTAB,0>; \
			friend I_Out<CONST,0>; \
			friend I_Out<MUTAB,1>; \
			friend I_Out<CONST,1>; \
			friend A_Verts<MUTAB>; \
			friend A_Verts<CONST>; \
			friend A_Edges<MUTAB>; \
			friend A_Edges<CONST>; \
			friend A_Outs<MUTAB,0>; \
			friend A_Outs<CONST,0>; \
			friend A_Outs<MUTAB,1>; \
			friend A_Outs<CONST,1>;


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

		using   H_Out  = typename Vert_Edges::Handle;
		using  SH_Out  = typename Vert_Edges::Handle_Small;
		using IDX_Out  = typename Vert_Edges::Index;


		struct H_Vert_Edge : Pair_Handle_Base< H_Vert_Edge, H_Vert, typename Vert_Edges::Handle > {
			using BASE = Pair_Handle_Base< H_Vert_Edge, H_Vert, typename Vert_Edges::Handle >;
			FORWARDING_CONSTRUCTOR(H_Vert_Edge, BASE) {}
		};




		// global edge
		struct Edge : Add_data<Edge_Data, Has_Edge_Data> {
			using BASE = Add_data<Edge_Data, Has_Edge_Data>;

			template<class... ARGS>
			Edge(IDX_Vert, IDX_Vert, ARGS&&... args) : BASE(std::forward<ARGS>(args)...) {}
		};



		struct Vert_Edge : Add_edge<SH_Edge, Has_Edge_Data || Edges_Global> {
			static constexpr bool Links_Vert_Edge = Outs_Link_Outs || Outs_Link_Ins;
			std::conditional_t<Links_Vert_Edge, H_Vert_Edge, H_Vert> link;

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







		struct Verts_Context;
		struct Edges_Context;
		template<int> struct Outs_Context;






		// Accessors forward declarations
		template<Const_Flag C> using A_Vert = typename Verts_Context::template Accessor<C>;
		template<Const_Flag C> using I_Vert = typename Verts_Context::template Iterator<C>;

		template<Const_Flag C> using A_Edge = typename Edges_Context::template Accessor<C>;
		template<Const_Flag C> using I_Edge = typename Edges_Context::template Iterator<C>;

		template<Const_Flag C, int oi_idx> using A_Out = typename Outs_Context<oi_idx>::template Accessor<C>;
		template<Const_Flag C, int oi_idx> using I_Out = typename Outs_Context<oi_idx>::template Iterator<C>;

		//template<Const_Flag C> using A_In  = typename Ins_Context::template Accessor<C>;
		//template<Const_Flag C> using I_In  = typename Ins_Context::template Iterator<C>;

		template<Const_Flag C> class A_Verts;
		template<Const_Flag C> class A_Edges;
		template<Const_Flag, int> class A_Outs;






		//
		// access/iterate verts
		//
		struct Verts_Context {
			using Container = Graph;
			using Handle = H_Vert;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Verts_Context> {
				using BASE = Accessor_Base<C,Verts_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				auto& graph()       { return _container; }
				auto& graph() const { return _container; }

				auto& data()       { return _container()._vs[ _handle() ].data; }
				auto& data() const { return _container()._vs[ _handle() ].data; }

				auto operator->()       { return &data(); }
				auto operator->() const { return &data(); }


				auto outs()       { return A_Outs<MUTAB,0>( _container(), _handle() ); }
				auto outs() const { return A_Outs<CONST,0>( _container(), _handle() ); }

				auto ins()       { return A_Outs<MUTAB,1>( _container(), _handle() ); }
				auto ins() const { return A_Outs<CONST,1>( _container(), _handle() ); }

				template<class X> auto out(const X& x)       { return outs()(x); }
				template<class X> auto out(const X& x) const { return outs()(x); }

				template<class X> auto in(const X& x)       { return ins()(x); }
				template<class X> auto in(const X& x) const { return ins()(x); }


				void erase() {
					static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
					static_assert(Verts_Erasable != NON_ERASABLE, "called erase() on non-verts-erasable graph");

					// remove edges
					if constexpr(Edges_Erasable != NON_ERASABLE) {
						if constexpr(Outs_Link_Outs) {
							for(auto& outt : _container()._vs[ _handle() ].outs()) {
								_container()._vs[outt().link.a].outs()(outt().link.b).erase();
							}
						}
						if constexpr(Outs_Link_Ins) {
							for(auto& outt : _container()._vs[ _handle() ].outs()) {
								_container()._vs[outt().link.a].ins()(outt().link.b).erase();
							}
							for(auto& outt : _container()._vs[ _handle() ].ins()) {
								_container()._vs[outt().link.a].outs()(outt().link.b).erase();
							}
						}
					}
					else {
						for( auto& outs : _container()._vs[_handle()].outs_ins ) {
							DCHECK( outs.empty() ) << "removing this vert removes edges, so requires Edges_Erasable";
						}
					}

					_container()._vs( _handle()).erase();
				}

			}; // Accessor


			struct End_Iterator {};


			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Verts_Context> {
				using BASE = Iterator_Base<C,Verts_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				GRAPH_FRIENDS

			private:
				friend BASE;

				void _increment() { _handle() = _container()._vs( _handle() ).next(); }
				void _decrement() { _handle() = _container()._vs( _handle() ).prev(); }

			public:
				bool operator!=(End_Iterator) const { return _handle() < _container()._vs.domain(); }
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
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				auto& graph()       {  return _container;  }
				auto& graph() const {  return _container;  }

				auto& data()       {  return _container()._es[ _handle() ].data;  }
				auto& data() const {  return _container()._es[ _handle() ].data;  }

				auto operator->()       { return &data(); }
				auto operator->() const { return &data(); }


				// TODO:
				//void erase() {
				//	static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
				//	_container()._es.erase( _handle );
				//}

			}; // Accessor


			struct End_Iterator {};


			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Edges_Context> {
				using BASE = Iterator_Base<C,Edges_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				GRAPH_FRIENDS

			private:
				friend BASE;
				void _increment() { _handle() = _container()._es( _handle() ).iterator().next(); }
				void _decrement() { _handle() = _container()._es( _handle() ).iterator().prev(); }

			public:
				bool operator!=(End_Iterator) const { return _handle() < _container()._es.domain(); }
			}; // Iterator

		}; // Verts_Context














		template<int oi>
		struct Outs_Context {
			using Container = Graph;
			using Handle = H_Vert_Edge;


			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Outs_Context> {
				using BASE = Accessor_Base<C,Outs_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				auto& graph()       {  return _container();  }
				auto& graph() const {  return _container();  }

				auto& data()       {  return _raw().data();  }
				auto& data() const {  return _raw().data();  }

				auto vert() { return _container().vert( _raw().vert() ); }
				auto edge() { return _container().edge( _raw().edge ); }


				// TODO
				//void erase() {
				//	static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
				//	if constexpr(Outs_Link_Outs) _container()._vs[ _raw().link.a ].outs( _raw().link.b ).erase();
				//	if constexpr(Outs_Link_Ins)  _container()._vs[ _raw().link.a ].ins( _raw().link.b ).erase();
				//	_container()._vs[ _handle().a ].outs( _handle().b ).erase();
				//}

			private:
				auto& _raw()       { return _raw_outs()[ _handle().b ]; }
				auto& _raw() const { return _raw_outs()[ _handle().b ]; }

				auto& _raw_outs()       { return _container()._vs[ _handle().a ].outs_ins[oi]; }
				auto& _raw_outs() const { return _container()._vs[ _handle().a ].outs_ins[oi]; }
			}; // Accessor


			struct End_Iterator {};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Outs_Context> {
				using BASE = Iterator_Base<C,Outs_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR( Iterator, BASE ) {}

			private:
				GRAPH_FRIENDS

			private:
				friend BASE;
				void _increment() {	_handle().b = _raw_outs()(_handle().b).next(); }
				void _decrement() { _handle().b = _raw_outs()(_handle().b).prev(); }

			private:
				auto& _raw_outs()       { return _container()._vs[ _handle().a ].outs_ins[oi]; }
				auto& _raw_outs() const { return _container()._vs[ _handle().a ].outs_ins[oi]; }

			public:
				bool operator!=(End_Iterator) const { return _handle().b != _raw_outs().domain(); }
			}; // Iterator

		}; // Outs_Context













		//
		// plural
		//
		template<Const_Flag C>
		class A_Verts {
		public:
			auto domain() const {  return _graph.vs.domain();  }

			auto empty() const {  return _graph._vs.empty();  }
			auto count() const {  return _graph._vs.count();  }

			void resize(int new_size) {
				if constexpr(Verts_Erasable != NON_ERASABLE) {
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

			auto end()       {  return typename Verts_Context::End_Iterator(); }
			auto end() const {  return typename Verts_Context::End_Iterator(); }


		private:
			A_Verts(Const<Graph,C>& graph) : _graph(graph) {}
			friend Graph;
			GRAPH_FRIENDS

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
					fr_out().link = H_Vert_Edge{ to, to_in };
					to_in().link  = H_Vert_Edge{ fr, fr_out };
					if constexpr(Has_Edge_Data || Edges_Global) {
						auto edge = _graph._es.add( fr, to, std::forward<ARGS>(args)... );
						fr_out().edge = edge;
						to_in().edge = edge;
					}
				}
				else if constexpr(!Directed) {
					auto fr_out = _graph._vs[fr].outs().add();
					auto to_out = _graph._vs[to].outs().add();
					fr_out().link = H_Vert_Edge{ to, to_out };
					to_out().link = H_Vert_Edge{ fr, fr_out };
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

			auto end()       {  return Edges_Context::End_Iterator(); }
			auto end() const {  return Edges_Context::End_Iterator(); }


		private:
			A_Edges(Const<Graph,C>& graph) : _graph(graph) {}
			friend Graph;
			GRAPH_FRIENDS

		private:
			Const<Graph,C>& _graph;
		};








		template<Const_Flag C, int oi>
		class A_Outs {
		public:
			auto domain() const { return _raw().domain(); }

			auto empty() const {  return _raw().empty();  }
			auto count() const {  return _raw().count();  }

			auto operator()(typename Vert_Edges::Index idx)       { return A_Out<C,oi>(&_graph, _vert, idx); }
			auto operator()(typename Vert_Edges::Index idx) const { return A_Out<C,oi>(&_graph, _vert, idx); }

			auto operator()(First_Tag)       { return A_Out<C,oi>(&_graph, H_Vert_Edge{_vert, _raw()(FIRST)}); }
			auto operator()(First_Tag) const { return A_Out<C,oi>(&_graph, H_Vert_Edge{_vert, _raw()(FIRST)}); }

			auto operator()(Last_Tag)        { return A_Out<C,oi>(&_graph, H_Vert_Edge{_vert, _raw()(LAST)}); }
			auto operator()(Last_Tag)  const { return A_Out<C,oi>(&_graph, H_Vert_Edge{_vert, _raw()(LAST)}); }


			auto begin()       { return I_Out<C,oi>(&_graph, H_Vert_Edge{_vert, _raw().begin()}); }
			auto begin() const { return I_Out<C,oi>(&_graph, H_Vert_Edge{_vert, _raw().begin()}); }

			auto end()         { return typename Outs_Context<oi>::End_Iterator(); }
			auto end() const   { return typename Outs_Context<oi>::End_Iterator(); }


		private:
			auto& _raw()       { return _graph._vs[_vert].outs_ins[oi]; }
			auto& _raw() const { return _graph._vs[_vert].outs_ins[oi]; }

		private:
			A_Outs(Const<Graph,C>& graph, H_Vert vert) : _graph(graph), _vert(vert) {}
			GRAPH_FRIENDS

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

			auto& operator[](H_Vert handle)       {  return A_Vert<MUTAB>( this, handle ).data();  }
			auto& operator[](H_Vert handle) const {  return A_Vert<CONST>( this, handle ).data();  }


			auto operator()(H_Edge handle)       {  return A_Edge<MUTAB>( this, handle );  }
			auto operator()(H_Edge handle) const {  return A_Edge<CONST>( this, handle );  }

			auto& operator[](H_Edge handle)       {  return A_Edge<MUTAB>( this, handle ).data();  }
			auto& operator[](H_Edge handle) const {  return A_Edge<CONST>( this, handle ).data();  }


		public:
			auto verts()       {  return A_Verts<MUTAB>( *this );  }
			auto verts() const {  return A_Verts<CONST>( *this );  }

			auto edges()       {  return A_Edges<MUTAB>( *this );  }
			auto edges() const {  return A_Edges<CONST>( *this );  }

		private:
			GRAPH_FRIENDS
		};









		struct With_Builder : Graph {
			FORWARDING_CONSTRUCTOR( With_Builder, Graph ) {}

			using DIRECTED = typename Context<true, Backlinks, Vert_Data, Edge_Data, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;

			using BACKLINKS = typename Context<Directed, true, Vert_Data, Edge_Data, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;

			template<class DATA>
			using VERT_DATA = typename Context<Directed, Backlinks, DATA, Edge_Data, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;

			template<class DATA>
			using EDGE_DATA = typename Context<Directed, Backlinks, Vert_Data, DATA, Verts_Erasable,
					Edges_Erasable, Edges_Global> ::With_Builder;


			using VERTS_ERASABLE = typename Context<Directed, Backlinks, Vert_Data, Edge_Data, ERASABLE_HOLES,
					Edges_Erasable, Edges_Global> ::With_Builder;

			using EDGES_ERASABLE = typename Context<Directed, Backlinks, Vert_Data, Edge_Data, Verts_Erasable,
					ERASABLE_HOLES, Edges_Global> ::With_Builder;


			using VERTS_ERASABLE_REORDER = typename Context<Directed, Backlinks, Vert_Data, Edge_Data, ERASABLE_REORDER,
					Edges_Erasable, Edges_Global> ::With_Builder;

			using EDGES_ERASABLE_REORDER = typename Context<Directed, Backlinks, Vert_Data, Edge_Data, Verts_Erasable,
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
	internal::graph::NON_ERASABLE, // verts erasable
	internal::graph::NON_ERASABLE, // edges erasable
	false // edges_global
> ::With_Builder;





} // namespace salgo