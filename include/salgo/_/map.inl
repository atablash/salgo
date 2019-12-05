#pragma once

#include "map.hpp"

#include "template-macros.hpp"

#include "iterable-base.hpp"

#include "helper-macros-on.inc"

namespace salgo::_::map {

template<class SUB_CONTAINER, class FUN>
struct Params {
	using SubContainer = SUB_CONTAINER;
	using Fun = FUN;

	// using SubHandle = typename CONTAINER::Handle;
	using Handle = typename SUB_CONTAINER::Handle;

	template<class C, SALGO_REQUIRES_CV_OF(C, Map<Params>)>
	static auto& underlying_cont(C& cont) { return cont._cont; }
};


// template<class P>
// class Handle {
// public:
// 	Handle(const typename P::SubHandle& subHandle) : _subHandle(subHandle) {}
// 	Handle(const Handle&) = default;
// 	Handle(Handle&&) = default;

// private:
// 	typename P::SubHandle _subHandle;
// 	friend Map<P>;
// };




template<class P, Const_Flag C>
class Reference : public Reference_Base<C, Context<P>> {
	using BASE = Reference_Base<C, Context<P>>;

	FORWARDING_CONSTRUCTOR(Reference, BASE),
		_cached_accessor( P::underlying_cont(CONT)(HANDLE) ) {}

protected:
	typename P::SubContainer::template Accessor<C> _cached_accessor;
};




template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C, Context<P>> {
	using BASE = Accessor_Base<C, Context<P>>;
	using BASE::BASE;
	friend Context<P>;

public:
	decltype(auto) erase() { return BASE::_cached_accessor.erase(); }
};



template<class P>
class End_Iterator {};



template<class P, Const_Flag C>
class Iterator : public Iterator_Base<C, Context<P>> {
	using BASE = Iterator_Base<C, Context<P>>;
	using BASE::BASE;
	friend Context<P>;

private:
	void _decrement() { --BASE::_cached_accessor.iterator(); MUT_HANDLE = BASE::_cached_accessor; }
	void _increment() { ++BASE::_cached_accessor.iterator(); MUT_HANDLE = BASE::_cached_accessor; }
	friend BASE;

public:
	bool operator!=(End_Iterator<P>) const { return BASE::_cached_accessor.iterator() != P::underlying_cont(CONT).end(); }
};



template<class P>
struct Context {
	using Container = Map<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Reference = map::Reference<P,C>;

	template<Const_Flag C>
	using Accessor = map::Accessor<P,C>;

	template<Const_Flag C>
	using Iterator = map::Iterator<P,C>;


	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
};





template<class P>
class Map : public ::salgo::Iterable_Base<Map<P>> {
	using SubContainer = typename P::SubContainer;
	using Fun = typename P::Fun;

public:
	using Handle = typename P::Handle;

	template<Const_Flag C> using Accessor = map::Accessor<P,C>;
	template<Const_Flag C> using Iterator = map::Iterator<P,C>;

public:
	Map(SubContainer& cont, Fun&& fun) : _cont(cont), _fun( std::forward<Fun>(fun) ) {}

	auto begin()       { return Context<P>::template create_iterator<MUTAB>(this, _cont.begin()); }
	auto begin() const { return Context<P>::template create_iterator<CONST>(this, _cont.begin()); }

	auto end() const { return End_Iterator<P>{}; }


	template<class H>
	decltype(auto) operator[](H&& handle)       { return _fun( _cont[ std::forward<H>(handle) ] ); }

	template<class H>
	decltype(auto) operator[](H&& handle) const { return _fun( _cont[ std::forward<H>(handle) ] ); }


	template<class H>
	decltype(auto) operator()(H&& handle)       { return Context<P>::template create_accessor<MUTAB>(this, std::forward<H>(handle)); }

	template<class H>
	decltype(auto) operator()(H&& handle) const { return Context<P>::template create_accessor<CONST>(this, std::forward<H>(handle)); }


	auto count() const { return _cont.count(); }
	auto domain() const { return _cont.domain(); }
	auto size() const { return _cont.size(); }

private:
	SubContainer& _cont;
	Fun _fun;
	friend P;
};

} // namespace salgo::internal::map

#include "helper-macros-off.inc"






namespace salgo {

template<class Container, class Fun>
class Map : public _::map::Map< _::map::Params< Container, Fun > > {
	using BASE = _::map::Map< _::map::Params< Container, Fun > >;

public:
	using BASE::BASE;
};

template<class Container, class Fun>
Map(Container, Fun) -> Map<Container, Fun>;

} // namespace salgo
