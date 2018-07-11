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


	using Block = typename salgo::Memory_Block<Val> ::CONSTRUCTED_FLAGS ::COUNT;


	using Handle       = typename Block::Handle;
	using Handle_Small = typename Block::Handle_Small;

	using Index = typename Block::Index;















	// forward
	class Vector_Allocator;
	using Container = Vector_Allocator;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

	private:
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		friend Vector_Allocator;

	public:
		void destruct() {
			static_assert(C == MUTAB, "called destruct() on CONST accessor");
			_container().v( _handle() ).destruct();
		}

		bool constructed() const { return _container().v( _handle() ).constructed(); }
		//bool exists_SLOW() const { return _container().v(h).exists_SLOW(); }
	};




	class End_Iterator {};



	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

	private:
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
		friend Vector_Allocator;

	private:
		friend Iterator_Base<C,Context>;

		void _increment() {
			do ++_handle(); while( *this != End_Iterator()  &&  !_container()( _handle() ).constructed() );
		}

		void _decrement() {
			do --_handle(); while( !_container().exists( _handle() ) );
		}

	public:
		bool operator!=(End_Iterator) const { return _handle() != _container().v.domain(); }
	};










	class Vector_Allocator {
		friend Accessor<CONST>;
		friend Accessor<MUTAB>;

		friend Iterator<CONST>;
		friend Iterator<MUTAB>;

	private:
		Block v;
		Index lookup_index = 0;

	public:
		using Val = Context::Val;
		using Handle_Small = Context::Handle_Small;
		using Handle       = Context::Handle;
		using Index        = Context::Index;

		static constexpr bool Auto_Destruct = true;

	public:
		Vector_Allocator() = default;

		// reserve and construct `num_starting_elements`
		template<class... ARGS>
		Vector_Allocator(int num_starting_elements, ARGS... args) :
				v(num_starting_elements),
				lookup_index(num_starting_elements) {
			for(int i=0; i<num_starting_elements; ++i) {
				v(i).construct( args... );
			}
		}


	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) {
			static_assert(std::is_move_constructible_v<Val>, "Vector_Allocator must be able to move its elements");

			// grow if needed
			if(v.count()*2 >= v.domain()) {
				//lookup_index = v.size();
				v.resize( v.domain()*2 + 1 );
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

			return Accessor<MUTAB>( this, v(index).construct( std::forward<ARGS>(args)... ).handle() );
		}

		template<class... ARGS>
		auto construct_near(Handle, ARGS&&... args) {
			//lookup_index = (int)h + 1;
			// TODO: currently we don't use hints, because this can kill performance if not done right
			return construct( std::forward<ARGS>(args)... );
		}

	public:
		auto& operator[]( Handle h )       { return v[h]; }
		auto& operator[]( Handle h ) const { return v[h]; }

		auto  operator()( Handle h )       { return Accessor<MUTAB>(this, h); }
		auto  operator()( Handle h ) const { return Accessor<CONST>(this, h); }


		auto& operator[]( First_Tag )       { return v[FIRST]; }
		auto& operator[]( First_Tag ) const { return v[FIRST]; }

		auto  operator()( First_Tag )       { return Accessor<MUTAB>(this, v(FIRST)); }
		auto  operator()( First_Tag ) const { return Accessor<CONST>(this, v(FIRST)); }


		auto& operator[]( Last_Tag )       { return v[LAST]; }
		auto& operator[]( Last_Tag ) const { return v[LAST]; }

		auto  operator()( Last_Tag )       { return Accessor<MUTAB>(this, v(LAST)); }
		auto  operator()( Last_Tag ) const { return Accessor<CONST>(this, v(LAST)); }



		auto count() const { return v.count(); }
		auto empty() const { return v.empty(); }

	public:
		auto begin()       { return Iterator<MUTAB>(this, v.begin()); }
		auto begin() const { return Iterator<MUTAB>(this, v.begin()); }

		auto end()       { return End_Iterator(); }
		auto end() const { return End_Iterator(); }
	};







	struct With_Builder : Vector_Allocator {

		FORWARDING_CONSTRUCTOR( With_Builder, Vector_Allocator ) {}

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL> :: With_Builder;

	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template< class VAL = int > // TODO: replace `int` with `void` and make it compile
using Vector_Allocator = typename internal::vector_allocator::Context<
	VAL
> :: With_Builder;








} // namespace salgo







