#pragma once

#include "reverse.hpp"

#include "template-macros.hpp"

#include "iterable-base.hpp"

#include "subscript-tags.hpp"

#include "helper-macros-on.inc"

namespace salgo::_::reverse {

template<class SUB_CONTAINER>
struct Params {
	using SubContainer = SUB_CONTAINER;

	// using SubHandle = typename CONTAINER::Handle;
	using Handle = typename SUB_CONTAINER::Handle;

	template<class C, SALGO_REQUIRES_CV_OF(C, Reverse<Params>)>
	static auto& underlying_cont(C& cont) { return cont._cont; }
};




template<class P, Const_Flag C>
class Reference : public Reference_Base<C, Context<P>> {
	using BASE = Reference_Base<C, Context<P>>;

public:
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
	void _decrement() { ++BASE::_cached_accessor.iterator(); MUT_HANDLE = BASE::_cached_accessor; }
	void _increment() { --BASE::_cached_accessor.iterator(); MUT_HANDLE = BASE::_cached_accessor; }
	friend BASE;

public:
	bool operator!=(End_Iterator<P>) const { return BASE::_cached_accessor.iterator() != P::underlying_cont(CONT).end(); }
};



template<class P>
struct Context {
	using Container = Reverse<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Reference = reverse::Reference<P,C>;

	template<Const_Flag C>
	using Accessor = reverse::Accessor<P,C>;

	template<Const_Flag C>
	using Iterator = reverse::Iterator<P,C>;


	template<Const_Flag C, class... ARGS>
	static auto create_accessor(ARGS&&... args) { return Accessor<C>( std::forward<ARGS>(args)... ); }

	template<Const_Flag C, class... ARGS>
	static auto create_iterator(ARGS&&... args) { return Iterator<C>( std::forward<ARGS>(args)... ); }
};





template<class P>
class Reverse : public ::salgo::Iterable_Base<Reverse<P>> {
	using SubContainer = typename P::SubContainer;

public:
	using Handle = typename P::Handle;

	template<Const_Flag C> using Accessor = reverse::Accessor<P,C>;
	template<Const_Flag C> using Iterator = reverse::Iterator<P,C>;

public:
	Reverse(SubContainer& cont) : _cont(cont) {}

	auto begin()       { return Context<P>::template create_iterator<MUTAB>(this, _cont(LAST)); }
	auto begin() const { return Context<P>::template create_iterator<CONST>(this, _cont(LAST)); }

	auto end() const { return End_Iterator<P>{}; }


	template<class H>
	decltype(auto) operator[](H&& handle)       { return _cont[ std::forward<H>(handle) ]; }

	template<class H>
	decltype(auto) operator[](H&& handle) const { return _cont[ std::forward<H>(handle) ]; }


	template<class H>
	decltype(auto) operator()(H&& handle)       { return Context<P>::template create_accessor<MUTAB>(this, std::forward<H>(handle)); }

	template<class H>
	decltype(auto) operator()(H&& handle) const { return Context<P>::template create_accessor<CONST>(this, std::forward<H>(handle)); }


	auto count() const { return _cont.count(); }
	auto domain() const { return _cont.domain(); }
	auto size() const { return _cont.size(); }

private:
	SubContainer& _cont;
	friend P;
};

} // namespace salgo::internal::reverse

#include "helper-macros-off.inc"






namespace salgo {

template<class Container>
class Reverse : public _::reverse::Reverse< _::reverse::Params< Container > > {
	using BASE = _::reverse::Reverse< _::reverse::Params< Container> >;

public:
	using BASE::BASE;
};

template<class Container>
Reverse(Container) -> Reverse<Container>;

} // namespace salgo
