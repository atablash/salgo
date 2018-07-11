#pragma once

#include "common.hpp"
#include "vector-allocator.hpp"


namespace salgo {


namespace internal {
namespace graph {



	//
	// add members
	//
	ADD_MEMBER(props);
	ADD_MEMBER(ins);
	ADD_MEMBER(_es);
	ADD_MEMBER(edge);




	/*
	enum class Erasable {
		NON_ERASABLE = 0,
		ERASABLE_WITH_HOLES = 1,
		ERASABLE_WITH_REORDER
	};
	namespace {
		constexpr auto NON_ERASABLE     = Erasable:: NON_ERASABLE;
		constexpr ERASABLE_WITH_HOLES   = Erasable:: ERASABLE_WITH_HOLES;
		constexpr ERASABLE_WITH_REORDER = Erasable:: ERASABLE_WITH_REORDER;
	}
	*/




	template<
		bool _DIRECTED,
		bool _BACKLINKS, // directed graphs can have 'ins' field
		class _VERT_PROPS,
		//class _OUT_PROPS,
		//class _IN_PROPS,
		class _EDGE_PROPS,
		//bool _VERTS_ERASABLE, // TODO
		//bool _EDGES_ERASABLE  // TODO
		bool _EDGES_GLOBAL
	>
	struct Context {

		static constexpr bool Directed = _DIRECTED;
		static constexpr bool Backlinks = _BACKLINKS;

		using Vert_Props      = _VERT_PROPS;
		using Edge_Props      = _EDGE_PROPS;

		static constexpr bool Edges_Global = _EDGES_GLOBAL;




		static constexpr bool Has_Vert_Props =
			!std::is_same_v<Vert_Props, void>;

		static constexpr bool Has_Edge_Props =
			!std::is_same_v<Edge_Props, void>;






		static constexpr bool Outs_Link_Outs = !Directed;
		static constexpr bool Outs_Link_Ins = Directed && Backlinks;




		#define GRAPH_FRIENDS \
			friend Graph; \
			friend A_Vert<MUTAB>; \
			friend A_Vert<CONST>; \
			friend A_Out<MUTAB>; \
			friend A_Out<CONST>; \
			friend A_Verts<MUTAB>; \
			friend A_Verts<CONST>; \
			friend A_Edges<MUTAB>; \
			friend A_Edges<CONST>; \
			friend A_Outs<MUTAB>; \
			friend A_Outs<CONST>; \


		class Graph;




		struct Vert;
		struct Vert_Edge;
		struct Edge;


		using Verts = salgo::Vector_Allocator< Vert >;
		using Edges = salgo::Vector_Allocator< Edge >;

		using Vert_Edges = salgo::Vector_Allocator< Vert_Edge >;


		using   H_Vert = typename Verts::Handle;
		using  SH_Vert = typename Verts::Handle_Small;
		using IDX_Vert = typename Verts::Index;

		using   H_Edge = typename Edges::Handle;
		using  SH_Edge = typename Edges::Handle_Small;
		using IDX_Edge = typename Edges::Index;


		struct H_Vert_Edge : Pair_Handle_Base< H_Vert_Edge, H_Vert, typename Vert_Edges::Handle > {
			using BASE = Pair_Handle_Base< H_Vert_Edge, H_Vert, typename Vert_Edges::Handle >;
			FORWARDING_CONSTRUCTOR(H_Vert_Edge, BASE) {}
		};




		// global edge
		struct Edge : Add_props<Edge_Props, Has_Edge_Props> {
		};



		struct Vert_Edge : Add_edge<SH_Edge, Has_Edge_Props || Edges_Global> {
			static constexpr bool Links_Vert_Edge = Outs_Link_Outs || Outs_Link_Ins;
			std::conditional_t<Links_Vert_Edge, H_Vert_Edge, H_Vert> link;

			auto& vert() { if constexpr(Links_Vert_Edge) return link.a; else return link; }
		};


		struct Vert :
				Add_props<Vert_Props, Has_Vert_Props>,
				Add_ins<Vert_Edges, Backlinks> {

			Vert_Edges outs;
		};







		struct Verts_Context;
		struct Edges_Context;
		struct Outs_Context;






		// Accessors forward declarations
		template<Const_Flag C> using A_Vert = typename Verts_Context::template Accessor<C>;
		template<Const_Flag C> using I_Vert = typename Verts_Context::template Iterator<C>;

		template<Const_Flag C> using A_Out = typename Outs_Context::template Accessor<C>;
		template<Const_Flag C> using I_Out = typename Outs_Context::template Iterator<C>;

		template<Const_Flag C> class A_Verts;
		template<Const_Flag C> class A_Edges;
		template<Const_Flag C> class A_Outs;






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
				auto& graph()       {  return _container;  }
				auto& graph() const {  return _container;  }

				auto& props()       {  return _container()._vs[ _handle() ].props;  }
				auto& props() const {  return _container()._vs[ _handle() ].props;  }

				auto outs()       { return A_Outs<MUTAB>( _container(), _handle() ); }
				auto outs() const { return A_Outs<CONST>( _container(), _handle() ); }

				template<class X> auto outs(const X& x)       { return outs()(x); }
				template<class X> auto outs(const X& x) const { return outs()(x); }


				// TODO:
				//void erase() {
				//	static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
				//	_container()._vs.erase( _handle );
				//}

			}; // Accessor



			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Verts_Context> {
				using BASE = Iterator_Base<C,Verts_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				GRAPH_FRIENDS

				// TODO: test incrementing handle in terms of vs subcontainer incrementation
				void _increment() {
					do ++_handle(); while((int)_handle() != _container->vs.domain() && !_container()._vs( _handle() ).constructed());
				}

				void _decrement() {
					do --_handle(); while(!_container()._vs( _handle ).constructed());
				}
			}; // Iterator

		}; // Verts_Context











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

				auto& props()       {  return _raw().props();  }
				auto& props() const {  return _raw().props();  }

				auto vert() { return _container().vert( _raw().vert() ); }


				// TODO
				//void erase() {
				//	static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
				//	if constexpr(Outs_Link_Outs) _container()._vs[ _raw().link.a ].outs( _raw().link.b ).erase();
				//	if constexpr(Outs_Link_Ins)  _container()._vs[ _raw().link.a ].ins( _raw().link.b ).erase();
				//	_container()._vs[ _handle().a ].outs( _handle().b ).erase();
				//}

			private:
				auto& _raw()       { return _container()._vs[ _handle().a ].outs[ _handle().b ]; }
				auto& _raw() const { return _container()._vs[ _handle().a ].outs[ _handle().b ]; }
			}; // Accessor



			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Outs_Context> {
				using BASE = Iterator_Base<C,Outs_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR( Iterator, BASE ) {}

			private:
				GRAPH_FRIENDS

				// TODO: test incrementing handle in terms of vs subcontainer incrementation
				void _increment() {
					do ++_handle().b; while((int)_handle().b != _raw_outs().domain() && !_raw_outs()( _handle().b ).constructed());
				}

				void _decrement() {
					do --_handle().b; while(!_raw_outs()( _handle().b ).constructed());
				}

			private:
				auto& _raw_outs() { return _container()._vs[ _handle().a ].outs; }
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

			auto begin()       {  return typename Verts_Context::template Iterator<C>    (&_graph, 0);  }
			auto begin() const {  return typename Verts_Context::template Iterator<CONST>(&_graph, 0);  }

			auto end()       {  return typename Verts_Context::template Iterator<C>    (&_graph, _graph.verts_domain()); }
			auto end() const {  return typename Verts_Context::template Iterator<CONST>(&_graph, _graph.verts_domain()); }


		private:
			A_Verts(Const<Graph,C>& graph) : _graph(graph) {}
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
			auto add(IDX_Vert fr, IDX_Vert to, ARGS... args) {
				if constexpr(Directed && !Backlinks) {
					auto& out = _graph._vs[fr].outs.construct( to )();
					if constexpr(Has_Edge_Props || Edges_Global) {
						out.edge = _graph._es.emplace( fr, to, std::forward<ARGS>(args)... );
					}
				}
				else if constexpr(Directed && Backlinks) {
					auto& fr_out = _graph._vs[fr].outs.construct()();
					auto& to_in  = _graph._vs[to].ins .construct()();
					fr_out.link = to_in;
					to_in.link = fr_out;
					if constexpr(Has_Edge_Props || Edges_Global) {
						auto edge = _graph._es.emplace( fr, to, std::forward<ARGS>(args)... );
						fr_out.edge = edge;
						to_in.edge = edge;
					}
				}
				else if constexpr(!Directed) {
					auto fr_out = _graph._vs[fr].outs.construct();
					auto to_out = _graph._vs[to].outs.construct();
					fr_out().link = H_Vert_Edge{ to, to_out };
					to_out().link = H_Vert_Edge{ fr, fr_out };
					if constexpr(Has_Edge_Props || Edges_Global) {
						auto edge = _graph._es.emplace( fr, to, std::forward<ARGS>(args)... );
						fr_out().edge = edge;
						to_out().edge = edge;
					}
				}
				else {
					DCHECK(false);
				}
			}

			//auto begin()       {  return typename Verts_Context::template Iterator<C>    (&_graph, 0);  }
			//auto begin() const {  return typename Verts_Context::template Iterator<CONST>(&_graph, 0);  }

			//auto end()       {  return typename Verts_Context::template Iterator<C>    (&_graph, _graph.verts_domain()); }
			//auto end() const {  return typename Verts_Context::template Iterator<CONST>(&_graph, _graph.verts_domain()); }


		private:
			A_Edges(Const<Graph,C>& graph) : _graph(graph) {}
			GRAPH_FRIENDS

		private:
			Const<Graph,C>& _graph;
		};








		template<Const_Flag C>
		class A_Outs {
		public:
			auto domain() const { return _raw().domain(); }

			auto empty() const {  return _raw().empty();  }
			auto count() const {  return _raw().count();  }

			auto operator()(typename Vert_Edges::Index idx)       { return A_Out<C>(&_graph, _vert, idx); }
			auto operator()(typename Vert_Edges::Index idx) const { return A_Out<C>(&_graph, _vert, idx); }

			auto operator()(First_Tag)       { return A_Out<C>(&_graph, _vert, _raw()(FIRST)); }
			auto operator()(First_Tag) const { return A_Out<C>(&_graph, _vert, _raw()(FIRST)); }

			auto operator()(Last_Tag)        { return A_Out<C>(&_graph, _vert, _raw()(LAST)); }
			auto operator()(Last_Tag)  const { return A_Out<C>(&_graph, _vert, _raw()(LAST)); }


			auto begin()       { return I_Out<C>(&_graph, _vert, 0); }
			auto begin() const { return I_Out<C>(&_graph, _vert, 0); }

			auto end()         { return I_Out<C>(&_graph, _vert, _raw().domain()); }
			auto end() const   { return I_Out<C>(&_graph, _vert, _raw().domain()); }


		private:
			auto& _raw()       { return _graph._vs[_vert].outs; }
			auto& _raw() const { return _graph._vs[_vert].outs; }

		private:
			A_Outs(Const<Graph,C>& graph, H_Vert vert) : _graph(graph), _vert(vert) {}
			GRAPH_FRIENDS

		private:
			Const<Graph,C>& _graph;
			H_Vert _vert;
		};









		class Graph :
				private Add__es<Edges, Has_Edge_Props> {

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

			auto operator()(H_Vert handle)       {  return A_Vert<MUTAB>( this, handle );  }
			auto operator()(H_Vert handle) const {  return A_Vert<CONST>( this, handle );  }

			auto operator[](H_Vert handle)       {  return A_Vert<MUTAB>( this, handle ).props();  }
			auto operator[](H_Vert handle) const {  return A_Vert<CONST>( this, handle ).props();  }


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
		};

	};

}
}




using Graph = internal::graph::Context<
	false, // directed
	false, // backlinks
	void, // vert props
	void, // edge props
	false // edges_global
> ::With_Builder;





} // namespace salgo
