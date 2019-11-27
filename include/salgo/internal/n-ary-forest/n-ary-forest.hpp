#pragma once

#include "accessor.hpp"

#include "../common.hpp"

#include "../inplace-storage.hpp"
#include "../const-flag.hpp"
#include "../vector-allocator.hpp"

#include <array>

#include "../helper-macros-on"

namespace salgo {
namespace internal {
namespace n_ary_forest {





template<class P, Const_Flag C>
class Accessor;

template<class P, Const_Flag C>
class Iterator;

template<class P>
class N_Ary_Forest;






ADD_MEMBER(data)
ADD_MEMBER(children)
ADD_MEMBER(parent)

template<class P>
struct Node :
		Add_data     <typename P::Data,         P::Has_Data>,
		Add_parent   <typename P::Parent,       P::Has_Parent_Links>,
		Add_children <typename P::Children,     P::Has_Child_Links>
		// Add_aggreg<Aggreg>,
		// Add_propag<Propag>
{
private:
	using DATA_BASE = Add_data     <typename P::Data,         P::Has_Data>;

public:
	using DATA_BASE :: DATA_BASE;
};





//
// Aggreg::Aggreg() { create neutral Aggreg; }
// Aggreg::aggregate(const Aggreg& other) { add other to this; }
//
//
// Propag::Propag() { create neutral Propag; }
// Propag::propagate(Propag& other) const { add this to other, assuming other's range is prefix of this; }
// Propag::apply(Aggreg& aggreg) {
//     add this to aggreg, assuming aggreg's range is prefix of this;
//     update this so it's a valid Propag for the range without 'aggreg' prefix;
// }
//



//
// add AGGREG
//
//ADD_MEMBER(aggreg)



//
// add PROPAG
//
//ADD_MEMBER(propag)












template<
	int N_ARY,
	class DATA,
	bool CHILD_LINKS,
	bool PARENT_LINKS,
	//bool EVERSIBLE,
	//class AGGREG, class PROPAG,
	class ALLOCATOR
>
struct Params {
	static constexpr auto N_Ary = N_ARY;
	using Data = DATA;
	static constexpr auto Has_Parent_Links = PARENT_LINKS;
	static constexpr auto Has_Child_Links = CHILD_LINKS;

	using Node = n_ary_forest::Node<Params>;

	using Allocator = typename ALLOCATOR ::template VAL<Node>; // rebind to `Node`

	static constexpr auto Has_Data = ! std::is_same_v<Data, void>;

	using Handle       = typename Allocator::Handle;
	using Handle_Small = typename Allocator::Handle_Small;

	using Parent = Handle_Small;
	using Children = std::conditional_t<
		N_Ary==0,
		std::vector<Handle_Small>,
		std::array<Handle_Small, N_Ary>
	>;

	template<Const_Flag C>
	using Accessor = n_ary_forest::Accessor<Params, C>;

	template<Const_Flag C>
	using Iterator = n_ary_forest::Iterator<Params, C>;
};





template<class PARAMS>
struct Context;






ADD_MEMBER(cached_parent)
ADD_MEMBER(cached_children)


template<class P, Const_Flag C>
class Reference : public Reference_Base<C, Context<P> >,
		private Add_cached_parent  <typename P::Parent,   P::Has_Parent_Links>,
		private Add_cached_children<typename P::Children, P::Has_Child_Links> {
	
	using CACHED_PARENT_BASE   = Add_cached_parent  <typename P::Parent,   P::Has_Parent_Links>;
	using CACHED_CHILDREN_BASE = Add_cached_children<typename P::Children, P::Has_Child_Links>;

	using BASE = Reference_Base<C, Context<P> >;
	using Node = n_ary_forest::Node<P>;

public:
	using BASE::BASE;

protected:
	bool just_erased = false;
	void on_erase() {
		if constexpr(P::Has_Parent_Links) {
			CACHED_PARENT_BASE::cached_parent = std::move( NODE.parent );
		}
		if constexpr(P::Has_Child_Links) {
			CACHED_CHILDREN_BASE::cached_children = std::move( NODE.children );
		}
		just_erased = true;
	}

	auto get_parent() const {
		static_assert(P::Has_Parent_Links);
		if(just_erased) return CACHED_PARENT_BASE::cached_parent;
		else return NODE.parent;
	}

	const auto& get_children() const {
		static_assert(P::Has_Child_Links);
		if(just_erased) return CACHED_CHILDREN_BASE::cached_children;
		else return NODE.children;
	}
};









template<class PARAMS>
class End_Iterator {};

//
// iterator
//
template<class PARAMS, Const_Flag C>
class Iterator : public Iterator_Base<C, Context<PARAMS> > {
	using BASE = Iterator_Base<C, Context<PARAMS> >;

public:
	FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

private:
	friend BASE;

	void _increment() {
		MUT_HANDLE = ++ALLOC( HANDLE ).iterator();
	}

	void _decrement() {
		MUT_HANDLE = --ALLOC( HANDLE ).iterator();
	}

public:
	bool operator!=(End_Iterator<PARAMS>) const { return ALLOC( HANDLE ).iterator() != ALLOC.end(); }
};










template<class P>
struct Context {
	using Container = N_Ary_Forest<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Reference = n_ary_forest::Reference<P, C>;

	template<Const_Flag C>
	using Accessor = n_ary_forest::Accessor<P, C>;

	template<Const_Flag C>
	using Iterator = n_ary_forest::Iterator<P, C>;
};



















template<class P>
class N_Ary_Forest : private P::Allocator, protected P {
public:
	using typename P::Allocator;
	using Handle       = typename P::Handle;
	using Handle_Small = typename P::Handle_Small;

	template<Const_Flag C>
	using Accessor = typename P::template Accessor<C>;

	template<Const_Flag C>
	using Iterator = typename P::template Iterator<C>;

private:
	auto& _alloc()       { return *static_cast<      Allocator*>(this); }
	auto& _alloc() const { return *static_cast<const Allocator*>(this); }

	friend Reference<P, MUTAB>;
	friend Reference<P, CONST>;

	friend Accessor<MUTAB>;
	friend Accessor<CONST>;

	friend Iterator<MUTAB>;
	friend Iterator<CONST>;

public:
	static_assert(Allocator::Auto_Destruct, "Rooted_Forest requires an Auto_Destruct allocator");

	bool empty() const {
		return _alloc().empty();
	}

	template<class... Args>
	auto emplace(Args&&... args) {
		auto new_node = _alloc().construct( std::forward<Args>(args)... );
		return Accessor<MUTAB>(this, new_node);
	}


public:
	auto& operator[](Handle handle)       { return _alloc()[handle].data; }
	auto& operator[](Handle handle) const { return _alloc()[handle].data; }

	auto operator()(Handle handle)       { return Accessor<MUTAB>(this, handle); }
	auto operator()(Handle handle) const { return Accessor<CONST>(this, handle); }

public:
	auto begin()       { return Iterator<MUTAB>(this, _alloc().begin()); }
	auto begin() const { return Iterator<CONST>(this, _alloc().begin()); }

	auto end() const { return End_Iterator<P>(); }
};



template<class P>
struct With_Builder : N_Ary_Forest<P> {
private:
	using BASE = N_Ary_Forest<P>;

	using BASE::BASE;

	using P::N_Ary;
	using typename P::Data;
	using P::Has_Child_Links;
	using P::Has_Parent_Links;
	using typename P::Allocator;

public:
	using CHILD_LINKS  = With_Builder< Params< N_Ary, Data, true,            Has_Parent_Links, Allocator > >;

	using PARENT_LINKS = With_Builder< Params< N_Ary, Data, Has_Child_Links, true,             Allocator > >;

	template<class X>
	using DATA         = With_Builder< Params< N_Ary, X,    Has_Child_Links, Has_Parent_Links, Allocator > >;

	template<class X>
	using ALLOCATOR    = With_Builder< Params< N_Ary, Data, Has_Child_Links, Has_Parent_Links, X         > >;

	template<int X>
	using ALIGN        = With_Builder< Params< N_Ary, Data, Has_Child_Links, Has_Parent_Links, typename Allocator ::template ALIGN<X> > >;
};




} // n_ary_forest
} // internal







// N_ARY is number of children per node
// if N_ARY==0, number of children is dynamic
template<int N_ARY, class DATA = void>
using N_Ary_Forest = internal::n_ary_forest::With_Builder< internal::n_ary_forest::Params <
	N_ARY,
	DATA,
	false, // child_links
	false, // parent_links
	Vector_Allocator<>
>>;



using Binary_Forest = N_Ary_Forest< 2 >;


using Rooted_Forest = N_Ary_Forest< 0 /* dynamic */>;







} // salgo

#include "../helper-macros-off"

