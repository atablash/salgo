#pragma once

#include "unordered-array.hpp"

#include "dynamic-array.inl" // instantiated as default parameter

#include "const-flag.hpp"
#include "accessors.hpp"
#include "handles.hpp"

#include <glog/logging.h>

#include "helper-macros-on.inc"

namespace salgo::_::unordered_array {



template<class VAL, class DYNAMIC_ARRAY>
struct Params {
	using Val = VAL;
	using Dynamic_Array = DYNAMIC_ARRAY;

	using Handle = unordered_array::Handle<Params>;
	using Index = unordered_array::Index<Params>;
};

template<class P>
struct Handle : Int_Handle_Base<Handle<P>,int> {
	using BASE = Int_Handle_Base<Handle<P>,int>;
	using BASE::BASE;
};


// same as Handle, but allow creation from `int`
template<class P>
struct Index : Handle<P> {
	using BASE = Handle<P>;
	using BASE::BASE;

	template<class... ARGS>
	Index(ARGS&&... args) : BASE(std::forward<ARGS>(args)...) {} // BASE::BASE(int) is explicit
};




// HACK to support erasing elements and back-iteration
template<class P, Const_Flag C>
struct End_Iterator : Iterator<P,C> {
	using BASE = Iterator<P,C>;
	using BASE::BASE;

	// forbid decrementing
private:
	using BASE::operator--;
	using BASE::operator-=;
	using BASE::operator++;
	using BASE::operator+=;
};







template<class P, Const_Flag C>
class Reference : public Reference_Base<C,Context<P>> {
	using BASE = Reference_Base<C,Context<P>>;
	
public:
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Reference, BASE) {}

protected:
	bool _just_erased = false;
};




template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C,Context<P>> {
	using BASE = Accessor_Base<C,Context<P>>;
	using BASE::BASE;
	friend Unordered_Array<P>;

	using BASE::_just_erased;

	using Val = typename P::Val;

public:
	using BASE::operator=;

public:
	int index() const { return BASE::handle(); }

	void erase() {
		static_assert(C == MUTAB, "called erase() on CONST accessor");
		DCHECK(!_just_erased);
		CONT[HANDLE].~Val();
		new(&CONT[HANDLE]) Val( std::move( CONT.back()()) );
		CONT.pop_back();
		_just_erased = true;
	}
};




template<class P, Const_Flag C>
class Iterator : public Iterator_Base<C,Context<P>> {
	using BASE = Iterator_Base<C,Context<P>>;
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
	friend Unordered_Array<P>;
	friend End_Iterator<P,C>;

	using BASE::_just_erased;

private:
	friend Iterator_Base<C,Context<P>>;

	inline void _increment() {
		if(!_just_erased) ++MUT_HANDLE;
		_just_erased = false;
	}
	inline void _decrement() {
		--MUT_HANDLE;
		_just_erased = false;
	}

	//
	// compare with End_Iterator
	//
public:
	template<Const_Flag CC>
	bool operator!=(const End_Iterator<P,CC>&) { return BASE::operator!=( CONT.end() ); }
	using BASE::operator!=; // still use normal version too
};



template<class P>
struct Context {
	using Container = Unordered_Array<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Reference = unordered_array::Reference<P,C>;

	template<Const_Flag C>
	using Accessor = unordered_array::Accessor<P,C>;

	template<Const_Flag C>
	using Iterator = unordered_array::Iterator<P,C>;
};








template<class P>
class Unordered_Array : protected P {
	using typename P::Index;

public:
	using typename P::Val;
	using typename P::Handle;

	template<Const_Flag C> using Accessor = Accessor<P,C>;
	template<Const_Flag C> using Iterator = Iterator<P,C>;

	//
	// data
	//
private:
	typename P::Dynamic_Array v;


	//
	// construction
	//
public:
	Unordered_Array() = default;
	Unordered_Array(int size) : v(size) {}

	template<class T>
	Unordered_Array(std::initializer_list<T>&& il) : v(std::move(il)) {}


	Unordered_Array(const Unordered_Array&) = default;
	Unordered_Array(Unordered_Array&&) = default;

	Unordered_Array& operator=(const Unordered_Array&) = default;
	Unordered_Array& operator=(Unordered_Array&&) = default;



	//
	// interface: manipulate element - can be accessed via the Accessor
	//
public:
	auto& operator[](Index handle)       { _check(handle); return v[handle]; }
	auto& operator[](Index handle) const { _check(handle); return v[handle]; }

	// inline void erase(Handle handle) = delete; // only through accessor




	//
	// interface
	//
public:
	auto operator()(Index handle)       { _check(handle); return _accessor(handle); }
	auto operator()(Index handle) const { _check(handle); return _accessor(handle); }

	template<class... ARGS>
	auto add(ARGS&&... args) {
		v.emplace_back( std::forward<ARGS>(args)... );
		return _accessor( v.size()-1 );
	}

	auto pop_back() {
		Val val = std::move( v[ v.size()-1 ] );
		v.pop_back();
		return val;
	}

	auto front()       { return _accessor( 0 ); }
	auto front() const { return _accessor( 0 ); }

	auto back()       { return _accessor( v.size()-1 ); }
	auto back() const { return _accessor( v.size()-1 ); }


	int size() const { return v.size(); }
	bool empty() const { return size() == 0; }

	void reserve(int capacity) { v.reserve(capacity); }



public:
	auto before_begin()       { return _accessor( -1 ).iterator(); }
	auto before_begin() const { return _accessor( -1 ).iterator(); }

	auto begin()       { return _accessor( 0 ).iterator(); }
	auto begin() const { return _accessor( 0 ).iterator(); }

	auto end()       { return End_Iterator<P,MUTAB>( this, Handle(v.size()) ); }
	auto end() const { return End_Iterator<P,CONST>( this, Handle(v.size()) ); }


private:
	auto _accessor(Index handle)       { return Accessor<MUTAB>(this, handle); }
	auto _accessor(Index handle) const { return Accessor<CONST>(this, handle); }

private:
	void _check(Handle handle) const {
		DCHECK( 0 <= handle && handle < v.size() ) << "index " << handle << " out of bounds";
	}

};






template<class P>
class With_Builder : public Unordered_Array<P> {
	using BASE = Unordered_Array<P>;

	using typename P::Val;
	using typename P::Dynamic_Array;

public:
	using BASE::BASE;

	template<int X>
	using INPLACE_BUFFER = With_Builder< Params< Val, typename Dynamic_Array :: template INPLACE_BUFFER<X>> >;
};




} // namespace salgo::_::Unordered_Array

#include "helper-macros-off.inc"


