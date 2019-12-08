#pragma once

/*

Allocates elements on a vector-like memory block.

Constructed objects are NOT persistent - they can be moved in memory by this allocator.

Grows memory block similar to std::vector, when congestion is 0.5 or more.

When constructing a new element, it looks for a hole in circular fashion.

Multithreaded code: keep in mind that old objects can be moved when new objects are allocated!

*/

#include "array-allocator.hpp"

#include "../accessors.hpp"
#include "../memory-block.inl"

#include "../subscript-tags.hpp"

#include "../helper-macros-on.inc"

namespace salgo::alloc::_::array_allocator {


template<
	class _VAL,
	int _ALIGN
>
struct Params {
	using Val = _VAL;
	static constexpr auto Align = _ALIGN;

	using Block = typename salgo::Memory_Block<Val> ::CONSTRUCTED_FLAGS ::COUNT ::template ALIGN<Align>;


	using Handle       = typename Block::Handle;
	using Handle_Small = typename Block::Handle_Small;
	using Index        = typename Block::Index;
};





//
// accessor
//
template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C,Context<P>> {
	using BASE = Accessor_Base<C,Context<P>>;

private:
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
	friend Array_Allocator<P>;

public:
	void construct() {
		static_assert(C == MUTAB, "called construct() on CONST accessor");
		CONT.v( HANDLE ).construct();
	}

	void destruct() {
		static_assert(C == MUTAB, "called destruct() on CONST accessor");
		CONT.v( HANDLE ).destruct();
	}

	void erase() { destruct(); } // alias

	bool constructed() const { return CONT.v( HANDLE ).constructed(); }
};





template<class P>
class End_Iterator {};



//
// iterator
//
template<class P, Const_Flag C>
class Iterator : public Iterator_Base<C,Context<P>> {
	using BASE = Iterator_Base<C,Context<P>>;

private:
	using BASE::BASE;
	friend Array_Allocator<P>;

private:
	friend Iterator_Base<C,Context<P>>;

	void _increment() {
		do ++MUT_HANDLE; while( *this != End_Iterator<P>()  &&  !CONT( HANDLE ).constructed() );
	}

	void _decrement() {
		do --MUT_HANDLE; while( !CONT().exists( HANDLE ) );
	}

public:
	bool operator!=(End_Iterator<P>) const { return HANDLE != CONT.v.domain(); }
};







template<class P>
struct Context {
	using Container = Array_Allocator<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Accessor = array_allocator::Accessor<P, C>;

	template<Const_Flag C>
	using Iterator = array_allocator::Iterator<P, C>;
};







template<class P>
class Array_Allocator : protected P {
	friend Accessor<P,CONST>;
	friend Accessor<P,MUTAB>;

	friend Iterator<P,CONST>;
	friend Iterator<P,MUTAB>;

public:
	using typename P::Val;
	using typename P::Handle_Small;
	using typename P::Handle;
	using typename P::Index;

	using P::Align;
	static constexpr bool Auto_Destruct = true;

private:
	typename P::Block v;
	Index lookup_index = 0;

public:
	Array_Allocator() = default;

	// reserve and construct `num_starting_elements`
	template<class... ARGS>
	Array_Allocator(int num_starting_elements, ARGS... args) :
			v(num_starting_elements),
			lookup_index(num_starting_elements) {
		for(int i=0; i<num_starting_elements; ++i) {
			v(i).construct( args... );
		}
	}



public:
	template<class... ARGS>
	auto construct(ARGS&&... args) {
		static_assert(std::is_move_constructible_v<Val>, "Array_Allocator must be able to move its elements");

		// grow if needed
		if(v.count()*2 >= v.domain()) {
			// std::cout << "grow array allocator from " << v.domain() << " to " << v.domain()*2+1 << std::endl;

			//lookup_index = v.size();
			v.resize( v.domain()*2 + 1 );

			// std::cout << "grow array allocator done" << std::endl;
		}

		while( v(lookup_index).constructed() ) {
			++lookup_index;
			if((int)lookup_index == v.domain()) {
				lookup_index = 0;
			}
		}

		auto index = lookup_index;

		++lookup_index;
		if(lookup_index == v.domain()) {
			lookup_index = 0;
		}

		return Accessor<P,MUTAB>( this, v(index).construct( std::forward<ARGS>(args)... ) );
	}

	template<class... ARGS>
	auto construct_near(Handle, ARGS&&... args) {
		//lookup_index = (int)h + 1;
		// TODO: currently we don't use hints, because this can kill performance if not done right
		return construct( std::forward<ARGS>(args)... );
	}

	template<class... ARGS>
	auto add(ARGS&&... args) { return construct( std::forward<ARGS>(args)... ); } // alias


	template<class... ARGS>
	void resize(int new_size, ARGS&&... args) {
		auto old_size = v.domain();
		v.resize(new_size);
		for(int i=old_size; i<new_size; ++i) {
			operator()(i).construct( args... );
		}
	}

	void reserve(int reserve_size) {
		if(reserve_size <= v.domain()) return;

		v.resize( reserve_size );
	}


public:
	auto& operator[]( Index h )       { return v[h]; }
	auto& operator[]( Index h ) const { return v[h]; }

	auto  operator()( Index h )       { return Accessor<P,MUTAB>(this, h); }
	auto  operator()( Index h ) const { return Accessor<P,CONST>(this, h); }


	auto& operator[]( First_Tag )       { return v[FIRST]; }
	auto& operator[]( First_Tag ) const { return v[FIRST]; }

	auto  operator()( First_Tag )       { return Accessor<P,MUTAB>(this, v(FIRST)); }
	auto  operator()( First_Tag ) const { return Accessor<P,CONST>(this, v(FIRST)); }


	auto& operator[]( Last_Tag )       { return v[LAST]; }
	auto& operator[]( Last_Tag ) const { return v[LAST]; }

	auto  operator()( Last_Tag )       { return Accessor<P,MUTAB>(this, v(LAST)); }
	auto  operator()( Last_Tag ) const { return Accessor<P,CONST>(this, v(LAST)); }


	auto count() const { return v.count(); }

	auto  is_empty() const { return v.is_empty(); }
	auto not_empty() const { return !is_empty(); }

	auto domain() const { return v.domain(); }

public:
	auto begin()       { return Iterator<P,MUTAB>(this, v.begin()); }
	auto begin() const { return Iterator<P,CONST>(this, v.begin()); }

	auto end() const { return End_Iterator<P>(); }
};






template<class P>
class With_Builder : public Array_Allocator<P> {
	using BASE = Array_Allocator<P>;

public:
	using BASE::BASE;

	using typename P::Val;
	using P::Align;

	template<class X>
	using VAL = With_Builder<Params<X, Align>>;

	template<int X>
	using ALIGN = With_Builder<Params<Val, X>>;
};



}  // namespace salgo::alloc::_::array_allocator



#include "../helper-macros-off.inc"



