#pragma once

#include "memory-block.hpp"
#include "vector.hpp"
#include "int-handle.hpp"
#include "global-instance.hpp"
#include "chunked-vector.hpp"
#include "accessors.hpp"







namespace xoroshiro128x {
	uint64_t s[2] = {1,1};

	static inline uint64_t rotl(const uint64_t x, int k) {
		return (x << k) | (x >> (64 - k));
	}

	uint64_t next(void) {
		const uint64_t s0 = s[0];
		uint64_t s1 = s[1];
		const uint64_t result = s0 + s1;

		s1 ^= s0;
		s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
		s[1] = rotl(s1, 36); // c

		return result;
	}


	/* This is the jump function for the generator. It is equivalent
	   to 2^64 calls to next(); it can be used to generate 2^64
	   non-overlapping subsequences for parallel computations. */

	void jump(void) {
		static const uint64_t JUMP[] = { 0xbeac0467eba5facb, 0xd86b048b86aa9922 };

		uint64_t s0 = 0;
		uint64_t s1 = 0;
		for(int i = 0; i < int(sizeof JUMP / sizeof *JUMP); i++)
			for(int b = 0; b < 64; b++) {
				if (JUMP[i] & UINT64_C(1) << b) {
					s0 ^= s[0];
					s1 ^= s[1];
				}
				next();
			}

		s[0] = s0;
		s[1] = s1;
	}
}

int rand_32() {
	auto x = xoroshiro128x::next() & 0x7fffffff;
	//std::cout << x << std::endl;
	return x;
}





namespace salgo {









namespace internal {
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


	using Vector = typename salgo::Chunked_Vector<Val> ::SPARSE ::COUNT;


	using       Handle = typename Vector::      Handle;
	using Small_Handle = typename Vector::Small_Handle;
















	// forward
	class Random_Allocator;
	using Container = Random_Allocator;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		friend Random_Allocator;

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
	private:
		Vector v;

	public:
		using Val = Context::Val;
		using Small_Handle = Context::Small_Handle;
		using       Handle = Context::      Handle;


	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) {
			if(v.domain() < v.capacity() || v.count()*2 >= v.domain()) {
				return Accessor<MUTAB>( this, v.emplace_back( std::forward<ARGS>(args)... ) );
			}

			for(;;) {
				DCHECK((v.domain() & (v.domain()+1)) == 0); // power of 2
				int idx = rand_32() & v.domain();
				for(int i=0; i<10; ++i) {
					int new_idx = idx + i;
					if(new_idx >= v.domain()) break;

					if(!v.exists(new_idx)) {
						v.construct(new_idx, std::forward<ARGS>(args)... );
						return Accessor<MUTAB>(this, new_idx);
					}
				}
			}
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







	class Allocator_Proxy {
		static auto& _get() { return global_instance<Random_Allocator>(); }

	public:
		using          Val = typename Random_Allocator::Val;
		using Small_Handle = typename Random_Allocator::Small_Handle;
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
	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template<
	class VAL
>
using Random_Allocator = typename internal::random_allocator::Context<
	VAL,
	false // singleton
>::With_Builder;








} // namespace salgo







