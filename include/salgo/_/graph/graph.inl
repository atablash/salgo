#pragma once

#include "graph.hpp"

#include "../alloc/array-allocator.inl" // default for vs
#include "../dynamic-array.inl" // default

#include "../alloc/array-allocator.hpp"

#include "../add-member.hpp"

#include "../subscript-tags.hpp"

#include <array>

#include "../helper-macros-on.inc"


namespace salgo::graph::_::graph {


SALGO_ADD_MEMBER(data);
SALGO_ADD_MEMBER(_es);
SALGO_ADD_MEMBER(edge);



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
struct Params {

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

	using Vert = graph::Vert<Params>;
	using Edge = graph::Edge<Params>;
	using Vert_Edge = graph::Vert_Edge<Params>;

	using Verts = std::conditional_t< Verts_Erasable == ERASABLE_HOLES,
			salgo::alloc::Array_Allocator< Vert >,
			std::conditional_t< Verts_Erasable == ERASABLE_REORDER,
				salgo::Dynamic_Array< Vert >,
				salgo::Dynamic_Array< Vert >
			>
	>;

	using Edges = std::conditional_t< Edges_Erasable == ERASABLE_HOLES,
		salgo::alloc::Array_Allocator< Edge >,
			std::conditional_t< Verts_Erasable == ERASABLE_REORDER,
					salgo::Dynamic_Array< Edge >,
					salgo::Dynamic_Array< Edge >
			>
	>;

	using Vert_Edges = std::conditional_t< Edges_Erasable == ERASABLE_HOLES,
		salgo::alloc::Array_Allocator< Vert_Edge >,
			std::conditional_t< Verts_Erasable == ERASABLE_REORDER,
					salgo::Dynamic_Array< Vert_Edge >,
					salgo::Dynamic_Array< Vert_Edge >
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
		using BASE::BASE;
	};

	struct H_Out : H_Vert_Edge_Base { using H_Vert_Edge_Base :: H_Vert_Edge_Base; };
	struct H_In  : H_Vert_Edge_Base { using H_Vert_Edge_Base :: H_Vert_Edge_Base; };

	template<int oi> using H_Vert_Edge = std::conditional_t<oi == 0, H_Out, H_In>;

	using Graph = graph::Graph<Params>;

	static auto& raw_vs(      Graph& g) {  return g._vs;  }
	static auto& raw_vs(const Graph& g) {  return g._vs;  }

	static auto& raw_es(      Graph& g) {  return g._es;  }
	static auto& raw_es(const Graph& g) {  return g._es;  }

	static auto& raw(      Graph& g, H_Vert h) {  return g._vs[h];  }
	static auto& raw(const Graph& g, H_Vert h) {  return g._vs[h];  }
}; // struct Params






// global edge
template<class P>
struct Edge : Add_data<typename P::Edge_Data, P::Has_Edge_Data> {
	using BASE = Add_data<typename P::Edge_Data, P::Has_Edge_Data>;

	using IDX_Vert = typename P::IDX_Vert;

	template<class... ARGS>
	Edge(IDX_Vert, IDX_Vert, ARGS&&... args) : BASE(std::forward<ARGS>(args)...) {}
};



template<class P>
struct Vert_Edge :
		Add_edge<typename P::SH_Edge, P::Has_Edge_Data || P::Edges_Global>,
		Add_data<typename P::Vert_Edge_Data, P::Has_Vert_Edge_Data> {
		
	static constexpr bool Links_Vert_Edge = P::Outs_Link_Outs || P::Outs_Link_Ins;
	std::conditional_t<Links_Vert_Edge,
		typename P::H_Vert_Edge_Base,
		typename P::H_Vert
	> link;

	auto& vert() { if constexpr(Links_Vert_Edge) return link.a; else return link; }
};


template<class P>
struct Vert :
		Add_data<typename P::Vert_Data, P::Has_Vert_Data> {

	std::array<typename P::Vert_Edges, P::Has_Ins ? 2 : 1> outs_ins;

	auto& outs()       { return outs_ins[0]; }
	auto& outs() const { return outs_ins[0]; }

	auto& ins()       { static_assert(P::Has_Ins); return outs_ins[1]; }
	auto& ins() const { static_assert(P::Has_Ins); return outs_ins[1]; }
};





template<class P>
struct Verts_Context {
	using Container = Graph<P>;
	using Handle = typename P::H_Vert;

	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Verts_Context> {
		using BASE = Accessor_Base<C,Verts_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

	public:
		auto& graph()       { return CONT; }
		auto& graph() const { return CONT; }

		auto outs()       { return A_Vert_Edges<P,C    ,0>( CONT, HANDLE ); }
		auto outs() const { return A_Vert_Edges<P,CONST,0>( CONT, HANDLE ); }

		auto ins()       { return A_Vert_Edges<P,C    ,1>( CONT, HANDLE ); }
		auto ins() const { return A_Vert_Edges<P,CONST,1>( CONT, HANDLE ); }

		template<class X> auto out(const X& x)       { return outs()(x); }
		template<class X> auto out(const X& x) const { return outs()(x); }

		template<class X> auto in(const X& x)       { return ins()(x); }
		template<class X> auto in(const X& x) const { return ins()(x); }


		void erase() {
			static_assert(P::Verts_Erasable != NOT_ERASABLE, "called erase() on non-verts-erasable graph");
			static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");

			// remove edges
			if constexpr(P::Edges_Erasable != NOT_ERASABLE) {
				if constexpr(P::Outs_Link_Outs) {
					for(auto& outt : P::raw_vs(CONT)[ HANDLE ].outs()) {
						P::raw_vs(CONT)[outt().link.a].outs()(outt().link.b).erase();
					}
				}
				if constexpr(P::Outs_Link_Ins) {
					for(auto& outt : P::raw_vs(CONT)[ HANDLE ].outs()) {
						CONT._vs[outt().link.a].ins()(outt().link.b).erase();
					}
					for(auto& outt : P::raw_vs(CONT)[ HANDLE ].ins()) {
						CONT._vs[outt().link.a].outs()(outt().link.b).erase();
					}
				}
			}
			else {
				for( auto& outs : P::raw_vs(CONT)[HANDLE].outs_ins ) {
					DCHECK( outs.empty() ) << "removing this vert removes edges, so requires Edges_Erasable";
				}
			}

			P::raw_vs(CONT)( HANDLE ).erase();
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
		bool operator!=(End_Iterator) const { return HANDLE < P::raw_vs(CONT).domain(); }
	}; // Iterator




	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }

}; // Verts_Context












template<class P>
struct Edges_Context {
	using Container = Graph<P>;
	using Handle = typename P::H_Edge;

	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Edges_Context> {
		using BASE = Accessor_Base<C,Edges_Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

	public:
		auto& graph()       {  return CONT;  }
		auto& graph() const {  return CONT;  }

		auto& data()       {  return P::raw_es(CONT)[ HANDLE ].data;  }
		auto& data() const {  return P::raw_es(CONT)[ HANDLE ].data;  }

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
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;
		void _increment() { HANDLE = CONT._es( HANDLE ).iterator().next(); }
		void _decrement() { HANDLE = CONT._es( HANDLE ).iterator().prev(); }

	public:
		bool operator!=(End_Iterator) const { return HANDLE < CONT._es.domain(); }
	}; // Iterator




	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
}; // Edges_Context














template<class P, int oi>
struct Vert_Edges_Context {
	using Container = Graph<P>;
	using Handle = typename P::template H_Vert_Edge<oi>;


	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Vert_Edges_Context> {
		using BASE = Accessor_Base<C,Vert_Edges_Context>;
		using BASE::BASE;

	public:
		auto& graph()       {  return CONT;  }
		auto& graph() const {  return CONT;  }


		auto& data() {
			if constexpr(P::Has_Vert_Edge_Data) return _raw().data();
			else DCHECK(false);
			return *this; // never reached
		}

		auto& data() const {
			if constexpr(P::Has_Vert_Edge_Data) return _raw().data();
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

		auto& _raw_outs()       { return P::raw_vs(CONT)[ HANDLE.a ].outs_ins[oi]; }
		auto& _raw_outs() const { return CONT._vs[ HANDLE.a ].outs_ins[oi]; }
	}; // Accessor


	struct End_Iterator {};

	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Vert_Edges_Context> {
		using BASE = Iterator_Base<C,Vert_Edges_Context>;
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
		friend Vert_Edges_Context;

	private:
		friend BASE;
		void _increment() {	MUT_HANDLE.b = _raw_outs()(HANDLE.b).iterator().next(); }
		void _decrement() { MUT_HANDLE.b = _raw_outs()(HANDLE.b).iterator().prev(); }

	private:
		auto& _raw_outs()       { return P::raw_vs(CONT)[ HANDLE.a ].outs_ins[oi]; }
		auto& _raw_outs() const { return P::raw_vs(CONT)[ HANDLE.a ].outs_ins[oi]; }

	public:
		bool operator!=(End_Iterator) const { return HANDLE.b != _raw_outs().domain(); }
	}; // Iterator





	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }


}; // Vert_Edges_Context











template<class P, Const_Flag C>
class A_Verts {
public:
	auto domain() const {  return P::raw_vs(_graph).domain();  }

	auto empty() const {  return P::raw_vs(_graph).empty();  }
	auto count() const {  return P::raw_vs(_graph).count();  }

	void resize(int new_size) {
		if constexpr(P::Verts_Erasable != NOT_ERASABLE) {
			for(int i=_graph._vs.domain()-1; i>=new_size; --i) {
				if(_graph.vert(i).found()) _graph.vert(i).erase();
			}
		}
		else {
			DCHECK(new_size > _graph._vs.domain()) << "can't shrink vertices list if not Verts_Erasable";
		}
		_graph._vs.resize(new_size);
	}


	auto begin()       {  return Verts_Context<P>::template create_iterator<C>    (&_graph, typename P::IDX_Vert(0));  }
	auto begin() const {  return Verts_Context<P>::template create_iterator<CONST>(&_graph, typename P::IDX_Vert(0));  }

	auto end() const {  return typename Verts_Context<P>::End_Iterator(); }


private:
	A_Verts(Const<Graph<P>,C>& graph) : _graph(graph) {}
	friend Graph<P>;

private:
	Const<Graph<P>,C>& _graph;
};





template<class P, Const_Flag C>
class A_Edges : private P {
	using typename P::IDX_Vert;
	using typename P::H_Vert_Edge_Base;

public:
	auto domain() const {  return _graph._es.domain();  }

	auto empty() const {  return _graph._es.empty();  }
	auto count() const {  return _graph._es.count();  }

	template<class... ARGS>
	auto add(IDX_Vert fr, IDX_Vert to, ARGS&&... args) {
		if constexpr(P::Directed && !P::Backlinks) {
			auto out = P::raw_vs(_graph)[fr].outs().add();
			out().link = to;
			if constexpr(P::Has_Edge_Data || P::Edges_Global) {
				out().edge = P::raw_es(_graph).add( fr, to, std::forward<ARGS>(args)... );
			}
		}
		else if constexpr(P::Directed && P::Backlinks) {
			auto fr_out = P::raw_vs(_graph)[fr].outs().add();
			auto to_in  = P::raw_vs(_graph)[to].ins().add();
			fr_out().link = H_Vert_Edge_Base{ to, to_in };
			to_in().link  = H_Vert_Edge_Base{ fr, fr_out };
			if constexpr(P::Has_Edge_Data || P::Edges_Global) {
				auto edge = _graph._es.add( fr, to, std::forward<ARGS>(args)... );
				fr_out().edge = edge;
				to_in().edge = edge;
			}
		}
		else if constexpr(!P::Directed) {
			auto fr_out = P::raw_vs(_graph)[fr].outs().add();
			auto to_out = P::raw_vs(_graph)[to].outs().add();
			fr_out().link = H_Vert_Edge_Base{ to, to_out };
			to_out().link = H_Vert_Edge_Base{ fr, fr_out };
			if constexpr(P::Has_Edge_Data || P::Edges_Global) {
				auto edge = P::raw_es(_graph).add( fr, to, std::forward<ARGS>(args)... );
				fr_out().edge = edge;
				to_out().edge = edge;
			}
		}
		else {
			DCHECK(false);
		}
	}

	auto begin()       {  return Edges_Context<P>::create_iterator<C>    (&_graph, 0);  }
	auto begin() const {  return Edges_Context<P>::create_iterator<CONST>(&_graph, 0);  }

	auto end() const {  return Edges_Context<P>::End_Iterator(); }


public:
	A_Edges(Const<Graph<P>,C>& graph) : _graph(graph) {}

private:
	Const<Graph<P>,C>& _graph;
};








template<class P, Const_Flag C, int oi>
class A_Vert_Edges {
public:
	auto domain() const { return _raw().domain(); }

	auto empty() const {  return _raw().empty();  }
	auto count() const {  return _raw().count();  }

	auto operator()(typename P::Vert_Edges::Index idx)       { return P::A_Vert_Edge<C,oi>(&_graph, _vert, idx); }
	auto operator()(typename P::Vert_Edges::Index idx) const { return P::A_Vert_Edge<C,oi>(&_graph, _vert, idx); }

	auto operator()(First_Tag)       { return Vert_Edges_Context<P,oi>::template create_accessor<C>(&_graph, typename P::template H_Vert_Edge<oi>{_vert, _raw()(FIRST)}); }
	auto operator()(First_Tag) const { return Vert_Edges_Context<P,oi>::template create_accessor<C>(&_graph, typename P::template H_Vert_Edge<oi>{_vert, _raw()(FIRST)}); }

	auto operator()(Last_Tag)        { return Vert_Edges_Context<P,oi>::template create_accessor<C>(&_graph, typename P::template H_Vert_Edge<oi>{_vert, _raw()(LAST)}); }
	auto operator()(Last_Tag)  const { return Vert_Edges_Context<P,oi>::template create_accessor<C>(&_graph, typename P::template H_Vert_Edge<oi>{_vert, _raw()(LAST)}); }


	auto begin()       { return Vert_Edges_Context<P,oi>::template create_iterator<C>(&_graph, typename P::template H_Vert_Edge<oi>{_vert, _raw().begin()}); }
	auto begin() const { return Vert_Edges_Context<P,oi>::template create_iterator<C>(&_graph, typename P::template H_Vert_Edge<oi>{_vert, _raw().begin()}); }

	auto end() const   { return typename Vert_Edges_Context<P,oi>::End_Iterator(); }


private:
	auto& _raw()       { return P::raw_vs(_graph)[_vert].outs_ins[oi]; }
	auto& _raw() const { return P::raw_vs(_graph)[_vert].outs_ins[oi]; }

public:
	A_Vert_Edges(Const<Graph<P>,C>& graph, typename P::H_Vert vert) : _graph(graph), _vert(vert) {}

private:
	Const<Graph<P>,C>& _graph;
	typename P::H_Vert _vert;
};








template<class P>
class Graph : protected P,
		private Add__es<typename P::Edges, P::Has_Edge_Data> {

	using ADD_ES_BASE = Add__es<typename P::Edges, P::Has_Edge_Data>;

	using typename P::IDX_Vert;
	using typename P::IDX_Edge;
	using typename P::H_In;
	using typename P::H_Out;

public:
	using H_Vert = typename P::H_Vert;
	using H_Edge = typename P::H_Edge;

private:
	typename P::Verts _vs;
	friend P;

public:
	Graph() {}
	Graph(int verts_size) : _vs(verts_size) {}

	//
	// accessors from handles
	//
public:
	auto vert(IDX_Vert handle)       {  return Verts_Context<P>::template create_accessor<MUTAB>( this, handle );  }
	auto vert(IDX_Vert handle) const {  return Verts_Context<P>::template create_accessor<CONST>( this, handle );  }

	auto vert(First_Tag)       { return vert( _vs(FIRST) ); }
	auto vert(First_Tag) const { return vert( _vs(FIRST) ); }

	auto vert(Last_Tag)       { return vert( _vs(LAST) ); }
	auto vert(Last_Tag) const { return vert( _vs(LAST) ); }



	auto edge(IDX_Edge handle)       {  return Edges_Context<P>::template create_accessor<MUTAB>( this, handle );  }
	auto edge(IDX_Edge handle) const {  return Edges_Context<P>::template create_accessor<CONST>( this, handle );  }

	auto edge(First_Tag)       { return edge( ADD_ES_BASE::_es(FIRST) ); }
	auto edge(First_Tag) const { return edge( ADD_ES_BASE::_es(FIRST) ); }

	auto edge(Last_Tag)       { return edge( ADD_ES_BASE::_es(LAST) ); }
	auto edge(Last_Tag) const { return edge( ADD_ES_BASE::_es(LAST) ); }





	auto operator()(H_Vert handle)       {  return Verts_Context<P>::template create_accessor<MUTAB>( this, handle );  }
	auto operator()(H_Vert handle) const {  return Verts_Context<P>::template create_accessor<CONST>( this, handle );  }


	template<class PP = P, class = std::enable_if_t< PP::Has_Vert_Data >>
	auto& operator[](H_Vert handle)       {  return _vs[handle].data;  }

	template<class PP = P, class = std::enable_if_t< PP::Has_Vert_Data >>
	auto& operator[](H_Vert handle) const {  return _vs[handle].data;  }





	auto operator()(H_Edge handle)       {  return Edges_Context<P>::template create_accessor<MUTAB>( this, handle );  }
	auto operator()(H_Edge handle) const {  return Edges_Context<P>::template create_accessor<CONST>( this, handle );  }

	template<class PP = P, class = std::enable_if_t< PP::Has_Edge_Data >>
	auto& operator[](H_Edge handle)       {  return ADD_ES_BASE::_es[handle].data;  }

	template<class PP = P, class = std::enable_if_t< PP::Has_Edge_Data >>
	auto& operator[](H_Edge handle) const {  return ADD_ES_BASE::_es[handle].data;  }





	auto operator()(H_Out handle)       {  return Vert_Edges_Context<P,0>::template create_accessor<MUTAB>( this, handle );  }
	auto operator()(H_Out handle) const {  return Vert_Edges_Context<P,0>::template create_accessor<MUTAB>( this, handle );  }

	template<class PP = P, class = std::enable_if_t< PP::Has_Vert_Edge_Data >>
	auto& operator[](H_Out handle)       {  return _vs[handle.a].outs[handle.b].data;  }

	template<class PP = P, class = std::enable_if_t< PP::Has_Vert_Edge_Data >>
	auto& operator[](H_Out handle) const {  return _vs[handle.a].outs[handle.b].data;  }





	auto operator()(H_In handle)       {  return Vert_Edges_Context<P,1>::template create_accessor<MUTAB>( this, handle );  }
	auto operator()(H_In handle) const {  return Vert_Edges_Context<P,1>::template create_accessor<MUTAB>( this, handle );  }

	template<class PP = P, class = std::enable_if_t< PP::Has_Vert_Edge_Data >>
	auto& operator[](H_In handle)       {  return _vs[handle.a].ins[handle.b].data;  }

	template<class PP = P, class = std::enable_if_t< PP::Has_Vert_Edge_Data >>
	auto& operator[](H_In handle) const {  return _vs[handle.a].ins[handle.b].data;  }


public:
	auto verts()       {  return A_Verts<P,MUTAB>( *this );  }
	auto verts() const {  return A_Verts<P,CONST>( *this );  }

	auto edges()       {  return A_Edges<P,MUTAB>( *this );  }
	auto edges() const {  return A_Edges<P,CONST>( *this );  }
};








template<class P>
struct With_Builder : Graph<P> {
	using BASE = Graph<P>;
	using BASE::BASE;

	using P::Directed;
	using P::Backlinks;
	using typename P::Vert_Data;
	using typename P::Edge_Data;
	using typename P::Vert_Edge_Data;
	using P::Verts_Erasable;
	using P::Edges_Erasable;
	using P::Edges_Global;

	using DIRECTED = With_Builder< Params<true, Backlinks, Vert_Data,
			Edge_Data, Vert_Edge_Data, Verts_Erasable,
			Edges_Erasable, Edges_Global>>;

	using BACKLINKS = With_Builder< Params<Directed, true, Vert_Data,
			Edge_Data, Vert_Edge_Data, Verts_Erasable,
			Edges_Erasable, Edges_Global>>;

	template<class DATA>
	using VERT_DATA = With_Builder< Params<Directed, Backlinks, DATA,
			Edge_Data, Vert_Edge_Data, Verts_Erasable,
			Edges_Erasable, Edges_Global>>;

	template<class DATA>
	using EDGE_DATA = With_Builder< Params<Directed, Backlinks, Vert_Data,
			DATA, Vert_Edge_Data, Verts_Erasable,
			Edges_Erasable, Edges_Global>>;

	template<class DATA>
	using VERT_EDGE_DATA = With_Builder< Params<Directed, Backlinks, Vert_Data,
			Edge_Data, DATA, Verts_Erasable,
			Edges_Erasable, Edges_Global>>;


	using VERTS_ERASABLE = With_Builder< Params<Directed, Backlinks, Vert_Data,
			Edge_Data, Vert_Edge_Data, ERASABLE_HOLES,
			Edges_Erasable, Edges_Global>>;

	using EDGES_ERASABLE = With_Builder< Params<Directed, Backlinks, Vert_Data,
			Edge_Data, Vert_Edge_Data, Verts_Erasable,
			ERASABLE_HOLES, Edges_Global>>;


	using VERTS_ERASABLE_REORDER = With_Builder< Params<Directed, Backlinks, Vert_Data,
			Edge_Data, Vert_Edge_Data, ERASABLE_REORDER,
			Edges_Erasable, Edges_Global>>;

	using EDGES_ERASABLE_REORDER = With_Builder< Params<Directed, Backlinks, Vert_Data,
			Edge_Data, Vert_Edge_Data, Verts_Erasable,
			ERASABLE_REORDER, Edges_Global>>;
	
};

} // namespace salgo::graph::_::graph

#include "../helper-macros-off.inc"


