#pragma once

#include "../memory-block.hpp"
#include "../dynamic-array.hpp"
#include "../handles.hpp"
#include "../global-instance.hpp"
#include "../chunked-array.hpp"
#include "../accessors.hpp"
#include "../rand.hpp"









#include "../helper-macros-on.inc"
namespace salgo {









namespace _ {
namespace random_allocator {







template<
	class _VAL,
	bool  _SINGLETON
>
struct Context {

	//
	// TEMPLATE PARAMETERS
	//
	using Val = _VAL;
	static constexpr bool Singleton = _SINGLETON;


	using Array = typename salgo::Chunked_Array<Val> ::SPARSE ::COUNT;


	using       Handle = typename Array::      Handle;
	using Handle_Small = typename Array::Handle_Small;
















	// forward
	class Random_Allocator;
	using Container = Random_Allocator;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		friend Random_Allocator;

	public:
		void destruct() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			CONT.v(HANDLE).destruct();
		}
	};



	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
		friend Random_Allocator;

		using BASE::_container;
		using BASE::_handle;


	private:
		friend Iterator_Base<C,Context>;

		void _increment() {
			do ++_handle; while( _handle != _container->end().accessor() && !_container->exists( _handle ) );
		}

		void _decrement() {
			do --_handle; while( !_container->exists( _handle ) );
		}

		auto _get_comparable() const {  return _handle;  }
	};










	class Random_Allocator {
		friend Accessor<CONST>;
		friend Accessor<MUTAB>;

		friend Iterator<CONST>;
		friend Iterator<MUTAB>;

	private:
		Array v;

	public:
		using Val = Context::Val;
		using Handle_Small = Context::Handle_Small;
		using       Handle = Context::      Handle;


	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) {
			// if current chunk not full yet, or congestion too high
			if(v.domain() < v.capacity() || v.count()*2 >= v.domain()) {
				// ...simply emplace_back a new element
				return Accessor<MUTAB>( this, v.emplace_back( std::forward<ARGS>(args)... ) );
			}

			// otherwise, find a hole in the chunked vector
			for(;;) {
				DCHECK((v.domain() & (v.domain()+1)) == 0); // power of 2

				// draw a random index, and check if it's free
				// check up to 10 subsequent elements before drawing another random index
				int idx = rand_32() & v.domain();
				for(int i=0; i<10; ++i) {
					int new_idx = idx + i;
					if(new_idx >= v.domain()) break;

					if(!v(new_idx).constructed()) {
						v(new_idx).construct( std::forward<ARGS>(args)... );
						return Accessor<MUTAB>(this, new_idx);
					}
				}
			}
		}

		template<class... ARGS>
		auto construct_near(Handle, ARGS&&... args) {
			// TODO
			// currently we don't use the hint
			return construct(std::forward<ARGS>(args)...);
		}

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







	class Allocator_Proxy {
		static auto& _get() { return global_instance<Random_Allocator>(); }

	public:
		using          Val = typename Random_Allocator::Val;
		using Handle_Small = typename Random_Allocator::Handle_Small;
		using       Handle = typename Random_Allocator::Handle;

	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) { return _get(). construct( std::forward<ARGS>(args)... ); }

		template<class... ARGS>
		auto  destruct(ARGS&&... args) { return _get().  destruct( std::forward<ARGS>(args)... ); }


		template<class... ARGS>
		auto& operator[](ARGS&&... args)       { return _get().operator[]( std::forward<ARGS>(args)... ); }
		
		template<class... ARGS>
		auto& operator[](ARGS&&... args) const { return _get().operator[]( std::forward<ARGS>(args)... ); }


		template<class... ARGS>
		auto operator()(ARGS&&... args)       { return _get().operator()( std::forward<ARGS>(args)... ); }
		
		template<class... ARGS>
		auto operator()(ARGS&&... args) const { return _get().operator()( std::forward<ARGS>(args)... ); }
	};






	using My_Allocator = std::conditional_t<
		Singleton,
		Allocator_Proxy,
		Random_Allocator
	>;




	struct With_Builder : My_Allocator {

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL, Singleton> :: With_Builder;

		using SINGLETON = typename
			Context<Val, true> :: With_Builder;

		// identity - it's always auto_destruct
		using AUTO_DESTRUCT = With_Builder;
	};


}; // struct Context
}  // namespace Storage
}  // namespace _







template<
	class VAL
>
using Random_Allocator = typename _::random_allocator::Context<
	VAL,
	false // singleton
>::With_Builder;








} // namespace salgo
#include "../helper-macros-off.inc"







