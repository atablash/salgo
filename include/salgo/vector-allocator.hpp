#pragma once

/*

Allocates elements on a vector-like memory block.

Constructed objects are NOT persistent - they can be moved in memory by this allocator.

Grows memory block similar to std::vector, when congestion is 0.5 or more.

When constructing a new element, it looks for a hole in circular fashion.

Multithreaded code: keep in mind that old objects can be moved when new objects are allocated!

*/

#include "memory-block.hpp"
#include "accessors.hpp"










namespace salgo {









namespace internal {
namespace vector_allocator {







template<
	class _VAL
>
struct Context {

	//
	// TEMPLATE PARAMETERS
	//
	using Val = _VAL;


	using Block = typename salgo::Memory_Block<Val> ::EXISTS ::COUNT;


	using       Handle = typename Block::      Handle;
	using Small_Handle = typename Block::Small_Handle;
















	// forward
	class Vector_Allocator;
	using Container = Vector_Allocator;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		friend Vector_Allocator;

	public:
		void destruct() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			BASE::_container->destruct( BASE::_handle );
		}
	};



	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
		friend Vector_Allocator;

		using BASE::_container;
		using BASE::_handle;


	private:
		friend Iterator_Base<C,Context>;

		void _increment() {
			do ++_handle(); while( _handle() != _container().end().accessor() && !_container().exists( _handle() ) );
		}

		void _decrement() {
			do --_handle(); while( !_container().exists( _handle() ) );
		}
	};










	class Vector_Allocator {
	private:
		Block v;
		Handle lookup_index = 0;

	public:
		using Val = Context::Val;
		using Small_Handle = Context::Small_Handle;
		using       Handle = Context::      Handle;

		static constexpr bool Auto_Destruct = true;


	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) {

			// grow if needed
			if(v.count()*2 >= v.size()) {
				//lookup_index = v.size();
				v.resize( v.size()*2 + 1 );
			}

			while( v(lookup_index).exists() ) {
				++lookup_index;
				if((int)lookup_index == v.size()) {
					lookup_index = 0;
				}
			}

			auto index = lookup_index;

			++lookup_index;
			if((int)lookup_index == v.size()) {
				lookup_index = 0;
			}

			return Accessor<MUTAB>( this, v.construct( index, std::forward<ARGS>(args)... ).handle() );
		}

		template<class... ARGS>
		auto construct_near(Handle, ARGS&&... args) {
			//lookup_index = (int)h + 1;
			// TODO: currently we don't use hints, because this can kill performance if not done right
			return construct( std::forward<ARGS>(args)... );
		}


		void destruct(Handle h ) { v.destruct(h); }

		auto& operator[]( Handle h )       { return v[h]; }
		auto& operator[]( Handle h ) const { return v[h]; }

		auto operator()( Handle h )       { return Accessor<MUTAB>(this, h); }
		auto operator()( Handle h ) const { return Accessor<CONST>(this, h); }

	public:
		auto begin()       { return v.begin(); }
		auto begin() const { return v.begin(); }

		auto end()       { return v.end(); }
		auto end() const { return v.end(); }
	};







	struct With_Builder : Vector_Allocator {

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL> :: With_Builder;

	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template< class VAL >
using Vector_Allocator = typename internal::vector_allocator::Context<
	VAL
> :: With_Builder;








} // namespace salgo







