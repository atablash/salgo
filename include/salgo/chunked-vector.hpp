#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "stack-storage.hpp"
#include "accessors.hpp"
#include "int-handle.hpp"

#include "memory-block.hpp"

#include "vector.hpp"



namespace salgo {






namespace internal {
namespace chunked_vector {




template<
	class VAL,
	bool SPARSE,
	class MEMORY_BLOCK
>
struct Params {};


namespace {
	inline int bit_scan_reverse(unsigned int x) { DCHECK_NE(0,x); return 31u ^ __builtin_clz(x); }

	inline bool is_power_of_2(unsigned int n) {
		DCHECK_NE(0, n) << "doesn't work for n==0";
		return (n & (n - 1)) == 0;
	}
}




template<class> struct       Handle;
template<class> struct Small_Handle;


template<class X>
struct Handle : Pair_Handle_Base<Handle<X>, Int_Handle<int>, int> { // for first number, char is enough
	using BASE = Pair_Handle_Base<Handle<X>, Int_Handle<int>, int>;

	template<class A, class B>
	Handle(A aa, B bb) : BASE(aa,bb) {}

	Handle(const Small_Handle<X>& small) {
		auto bsr = bit_scan_reverse(small.a);
		BASE::a = bsr;
		BASE::b = (1u<<bsr) ^ small.a;
	}

	// convert from/to array index
	Handle(unsigned int index) : Handle(Small_Handle<X>(index)) {}

	// iteration
	Handle& operator++() {
		if( ++BASE::b == (1 << BASE::a) ) {
			++BASE::a;
			BASE::b = 0;
		}
		return *this;
	}

	Handle& operator--() {
		if( BASE::b == 0 ) {
			--BASE::a;
			BASE::b = (1 << BASE::a);
		}
		--BASE::b;
		return *this;
	}
};




template<class X>
struct Small_Handle : Int_Handle_Base<Small_Handle<X>, unsigned int, 0> {
	using BASE = Int_Handle_Base<Small_Handle<X>, unsigned int, 0>;
	//FORWARDING_CONSTRUCTOR(Small_Handle, BASE);

	Small_Handle(const Handle<X>& big) : BASE(1<<big.a ^ big.b) {
		DCHECK_LT(big.a, 32);
		DCHECK_LT(big.b, 1<<big.a);
	}

	//operator Handle<X>() const {
	//	auto bsr = bit_scan_reverse(BASE::a);
	//	return Handle<X>(bsr, (1<<bsr) ^ BASE::a);
	//}

	// convert from/to array index
	Small_Handle(unsigned int x) : BASE(x+1) {}
	operator unsigned int() const { return BASE::a-1; }
};







template<class _VAL, bool _SPARSE, class _MEMORY_BLOCK>
struct Context {

	using P = Params<_VAL, _SPARSE, _MEMORY_BLOCK>;

	//
	// forward declarations
	//
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Chunked_Vector;
	using Container = Chunked_Vector;





	using Val = _VAL;
	using Memory_Block = _MEMORY_BLOCK;


	static constexpr bool Exists_Inplace       = _MEMORY_BLOCK :: Has_Exists_Inplace;
	static constexpr bool Exists_Chunk_Bitset  = _MEMORY_BLOCK :: Has_Exists_Bitset;
	static constexpr bool Exists               = _MEMORY_BLOCK :: Has_Exists;

	static constexpr bool Sparse         = _SPARSE;
	static constexpr bool Dense          = !Sparse;

	static constexpr bool Iterable = Dense || Exists;


	using       Handle = chunked_vector::      Handle<P>;
	using Small_Handle = chunked_vector::Small_Handle<P>;


	static constexpr bool Has_Pop_Back_Result = std::is_move_constructible_v<Val>;
	using Pop_Back_Result = std::conditional_t<Has_Pop_Back_Result, Val, void>;








	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Accessor,BASE) {}
		friend Chunked_Vector;

		using BASE::_container;
		using BASE::_handle;


	public:
		using BASE::operator=;

		template<class... ARGS>
		void construct(ARGS&&... args) {
			static_assert(C == MUTAB, "called construct() on CONST accessor");
			_container->construct( _handle, std::forward<ARGS>(args)... );
		}

		void destruct() {
			static_assert(C == MUTAB, "called destruct() on CONST accessor");
			_container->destruct( _handle );
		}

		void erase() {
			destruct();
		}

		bool exists() const {
			return _container->exists( _handle );
		}

	};


	//
	// accessor
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Iterator,BASE) {}
		friend Chunked_Vector;

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














	class Chunked_Vector {

	public:
		using Val = Context::Val;
		using Handle = Context::Handle;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;


		//
		// data
		//
	private:
		salgo::Vector< Memory_Block > _blocks;
		int _size = 0;


		//
		// construction
		//
	public:
		Chunked_Vector() = default;

		Chunked_Vector(int new_size) { resize(new_size); }

		template<class T>
		Chunked_Vector(std::initializer_list<T>&& l) {
			static_assert(std::is_constructible_v<Val, const T&&>, "wrong initializer_list element type");

			reserve( l.size() );
			for(auto&& e : l) {
				emplace_back( std::move(e) );
			}
		}

		Chunked_Vector(const Chunked_Vector&) = default;
		Chunked_Vector(Chunked_Vector&&) = default;

		Chunked_Vector& operator=(const Chunked_Vector&) = default;
		Chunked_Vector& operator=(Chunked_Vector&&) = default;

		//
		// element interface
		//
	public:
		auto& operator[](Handle handle) {
			_check_bounds(handle);
			return _blocks[handle.a][handle.b];
		}

		auto& operator[](Handle handle) const {
			_check_bounds(handle);
			return _blocks[handle.a][handle.b];
		}




		auto operator()(Handle handle) {
			_check_bounds(handle);
			return Accessor<MUTAB>(this, handle);
		}

		auto operator()(Handle handle) const {
			_check_bounds(handle);
			return Accessor<CONST>(this, handle);
		}




		template<class... ARGS>
		void construct(Handle handle, ARGS&&... args) {  static_assert(Sparse);
			_check_bounds(handle);
			_blocks[handle.a].construct( handle.b, std::forward<ARGS>(args)... );
		}

		void destruct(Handle handle) {  static_assert(Sparse);
			_check_bounds(handle);
			_blocks[handle.a].destruct( handle.b );
		}

		bool exists(Handle handle) const {
			_check_bounds(handle);
			if constexpr(Dense) return true;
			if constexpr(Exists_Chunk_Bitset || Exists_Inplace) return _blocks[handle.a].exists( handle.b );
			// TODO: global bitset
		}




	private:
		void _check_bounds(Handle handle) const {
			DCHECK_LT(handle.a, _blocks.size());
			DCHECK_LT(handle.b, _blocks[handle.a].size());
		}

		//
		// interface
		//
	public:
		template<class... ARGS>
		Accessor<MUTAB> emplace_back(ARGS&&... args) {
			static_assert(std::is_constructible_v<Val, ARGS...>);

			++_size;
			if(is_power_of_2(_size)) {
				_blocks.emplace_back( _size );
			}

			auto idx = _size - _blocks.back()().size();
			_blocks.back()().construct( idx, std::forward<ARGS>(args)... );

			return Accessor<MUTAB>( this, Handle(_blocks.size()-1, idx) );
		}

		Pop_Back_Result pop_back() {
			static_assert(Dense || Exists, "can't pop_back() if last element is unknown");

			DCHECK_GT(_size, 0) << "pop_back() on empty Vector";

			auto idx = _size - _blocks.back()().size();

			if constexpr(Exists_Chunk_Bitset || Exists_Inplace) {
				while(!_blocks.back()().exists(idx)) {
					if(idx == 0) {
						idx = _size >> 1;
						_blocks.pop_back();
					}
					--idx;
					--_size;
					DCHECK_GT(_size, 0) << "pop_back() on empty Vector";
					DCHECK_GT(_blocks.size(), 0);
				}
			}
			DCHECK( exists(_size-1) );

			if constexpr(Has_Pop_Back_Result) {
				Val result = std::move( _blocks.back()()[idx] );
				_blocks.back()().destruct(idx);
				if(idx == 0) {
					_blocks.pop_back();
				}
				--_size;

				return result;
			}
			else {
				_blocks.back()().destruct(idx);
				if(idx == 0) {
					_blocks.pop_back();
				}
				--_size;
			}
		}


		int size() const {
			static_assert(Dense, "size() for sparse vector is a bit ambiguous. Use count() or domain() instead");
			return _size;
		}

		template<class... ARGS>
		void resize(int new_size, ARGS&&... args) {
			if(new_size == 0) {
				_blocks.clear();
				_size = 0;
				return;
			}

			// destruct elements
			for(int i=new_size; i<_size; ++i ) {
				auto h = Handle(i);
				if( exists(h) ) _blocks[h.a].destruct(h.b);
			}

			int new_num_blocks = bit_scan_reverse(new_size) + 1;

			// construct blocks
			if(new_num_blocks > _blocks.size()) {
				int curr_size = _blocks.empty() ? 1 : _blocks.back()().size()*2;
				_blocks.reserve( new_num_blocks );
				for(int i=_blocks.size(); i<new_num_blocks; ++i) {
					_blocks.emplace_back( curr_size );
					curr_size *= 2;
				}
			}
			else {
				// destruct blocks
				_blocks.resize( new_num_blocks );
			}
			

			// construct elements
			for(int i=_size; i<new_size; ++i) {
				auto h = Handle(i);
				_blocks[h.a].construct(h.b, args...);
			}

			_size = new_size;
		}

		void clear() { resize(0); }

		int capacity() const {
			if(_blocks.empty()) return 0;

			return _blocks.back()().size()*2 - 1;
		}

		// currently reserves only space for _blocks
		void reserve(int new_capacity) {
			_blocks.reserve( bit_scan_reverse(new_capacity) + 1 );
		}



	public:
		inline auto begin() {
			static_assert(Iterable);
			auto e = (*this)(0).iterator();
			if(!e->exists()) ++e;
			return e;
		}

		inline auto begin() const {
			static_assert(Iterable);
			auto e = (*this)(0).iterator();
			if(!e->exists()) ++e;
			return e;
		}



		inline auto end() {
			static_assert(Iterable);
			return Iterator<MUTAB>(this, _size);
		}

		inline auto end() const {
			static_assert(Iterable);
			return Iterator<CONST>(this, _size);
		}

	};







	struct With_Builder : Chunked_Vector {
		FORWARDING_CONSTRUCTOR(With_Builder, Chunked_Vector) {}


		using EXISTS =
			typename Context< Val, Sparse, typename Memory_Block::EXISTS > :: With_Builder;

		using EXISTS_INPLACE =
			typename Context< Val, Sparse, typename Memory_Block::EXISTS_INPLACE > :: With_Builder;

		using EXISTS_CHUNK_BITSET =
			typename Context< Val, Sparse, typename Memory_Block::EXISTS_BITSET > :: With_Builder;



		// also enable EXISTS by default
		using SPARSE =
			typename Context< Val, true, Memory_Block > :: With_Builder :: EXISTS;


		// just enable SPARSE, but no EXISTS
		using SPARSE_NO_EXISTS =
			typename Context< Val, true, Memory_Block > :: With_Builder;

		// TODO: COUNT
	};




}; // struct Context
} // namespace Chunked_Vector
} // namespace internal






template< class T >
using Chunked_Vector = typename internal::chunked_vector::Context<
	T,
	false, // SPARSE
	Memory_Block<T>
> :: With_Builder;











}
