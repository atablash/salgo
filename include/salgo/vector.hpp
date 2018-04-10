#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "stack-storage.hpp"
#include "iterator-base.hpp"
#include "int-handle.hpp"

#include "memory-block.hpp"



namespace salgo {






namespace internal {
namespace Vector {


struct Handle : Int_Handle<int,Handle> {
	using BASE = Int_Handle<int,Handle>;
	FORWARDING_CONSTRUCTOR(Handle, BASE);
};


template<class _VAL, bool _SPARSE, class _MEMORY_BLOCK>
struct Context {

	//
	// forward declarations
	//
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Vector;





	using Val = _VAL;
	using Memory_Block = _MEMORY_BLOCK;

	using Allocator = typename _MEMORY_BLOCK :: Allocator;
	static constexpr int  Stack_Buffer   = _MEMORY_BLOCK :: Stack_Buffer;
	static constexpr bool Exists_Inplace = _MEMORY_BLOCK :: Has_Exists_Inplace;
	static constexpr bool Exists_Bitset  = _MEMORY_BLOCK :: Has_Exists_Bitset;
	static constexpr bool Exists         = _MEMORY_BLOCK :: Has_Exists;
	static constexpr bool Count          = _MEMORY_BLOCK :: Has_Count;

	static constexpr bool Sparse         = _SPARSE;
	static constexpr bool Dense          = !Sparse;

	using Handle = internal::Vector::Handle;











	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		Handle handle() const {
			return _key;
		}

		auto& operator()()       { return val(); }
		auto& operator()() const { return val(); }

		Const<Val,C>& val() {
			if constexpr(Exists) DCHECK( _owner.exists( _key ) ) << "accessing erased element";
			return _owner[ _key ];
		}

		const Val& val() const {
			if constexpr(Exists) DCHECK( _owner.exists( _key ) ) << "accessing erased element";
			return _owner[ _key ];
		}

		template<class... ARGS>
		void construct(ARGS&&... args) {
			static_assert(C == MUTAB, "called construct() on CONST accessor");
			_owner.construct( _key, std::forward<ARGS>(args)... );
		}

		void destruct() {
			static_assert(C == MUTAB, "called destruct() on CONST accessor");
			_owner.destruct( _key );
		}

		bool exists() const {
			return _owner.exists( _key );
		}


	private:
		Accessor(Const<Vector,C>& owner, Handle key)
			: _owner(owner), _key(key) {}

		friend Vector;
		friend Iterator<C>;


	private:
		Const<Vector,C>& _owner;
		const Handle _key;
	};






	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Iterator> {

		// member functions accessed by BASE:
	private:
		friend Iterator_Base<C,Iterator>;

		void _increment() {
			do ++_key; while( _key != _owner.domain() && !_owner.exists( _key ) );
		}

		void _decrement() {
			do --_key; while( !_owner.exists( _key ) );
		}

		auto _get_comparable() const {  return _key;  }

		template<Const_Flag CC>
		auto _will_compare_with(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
		}



	public:
		auto operator*() const {  return Accessor<C>(_owner, _key);  }

		// unable to implement if using accessors:
		// auto operator->()       {  return &container[idx];  }




	private:
		Iterator(Const<Vector,C>& owner, Handle key)
				: _owner(owner), _key(key) {
			if( (int)key != owner.domain() && !owner.exists(key) ) _increment();
		}

		friend Vector;

	private:
		Const<Vector,C>& _owner;
		int _key;
	};








	class Vector {

	public:
		using Val = Context::Val;
		static constexpr bool Has_Exists_Inplace = Context::Exists_Inplace;
		static constexpr bool Has_Exists_Bitset = Context::Exists_Bitset;
		static constexpr bool Has_Exists = Context::Exists;
		static constexpr bool Has_Count = Context::Count;

		using Handle = Context::Handle;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;

		friend Iterator<MUTAB>;
		friend Iterator<CONST>;


		//
		// data
		//
	private:
		Memory_Block _mb;
		int _size = 0;


		//
		// construction
		//
	public:
		Vector() = default;

		template<class... ARGS>
		Vector(int size, ARGS&&... args) : _mb(size), _size(size) {
			for(int i=0; i<size; ++i) {
				_mb.construct(i, args...);
			}
		}

		~Vector() {
			if constexpr(Dense) {
				for(int i=0; i<_size; ++i) {
					_mb.destruct(i);
				}
			}
		}



		template<class... ARGS>
		void resize(int new_size, ARGS&&... args) {
			if constexpr(Dense) {
				_mb.resize( new_size, [this](int i){return i<_size;} );
			}
			else {
				_mb.resize( new_size );
			}

			// construct new elements
			for(int i=_size; i<_mb.size(); ++i) {
				_mb.construct(i, args...);
			}

			_size = new_size;
		}


		void reserve(int capacity) {
			_mb.resize( capacity );
		}



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		template<class... ARGS>
		void construct(Handle key, ARGS&&... args) {  static_assert(Sparse);
			_check_bounds(key);
			_mb.construct( key, std::forward<ARGS>(args)... );
		}

		void destruct(Handle key) {  static_assert(Sparse);
			_check_bounds(key);
			_mb.destruct( key );
		}

		bool exists(Handle key) const {
			_check_bounds(key);
			if constexpr(Dense) return true;
			else return _mb.exists( (int)key );
		}

		Val& operator[](Handle key) {
			_check_bounds(key);
			return _mb[ (int)key ];
		}

		const Val& operator[](Handle key) const {
			_check_bounds(key);
			return _mb[ (int)key ];
		}





		//
		// interface
		//
	public:
		auto operator()(Handle key) {
			_check_bounds(key);
			return Accessor<MUTAB>(*this, key);
		}

		auto operator()(Handle key) const {
			_check_bounds(key);
			return Accessor<CONST>(*this, key);
		}

	private:
		inline void _check_bounds(Handle key) const {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, _size ) << "index out of bounds";
		}





	public:
		template<class... ARGS>
		Accessor<MUTAB> emplace_back(ARGS&&... args) {
			// because of vector realloc:
			static_assert( Dense || Exists || std::is_trivially_move_constructible_v<Val>,
				"non-trivially-constructible types require EXISTS to emplace_back()" );

			if(_size == _mb.size()) {
				if constexpr(Dense) {
					_mb.resize( (_mb.size() + 1) * 3/2, [](int){ return /*i<_size*/true; } );
				}
				else {
					_mb.resize( (_mb.size() + 1) * 3/2 );
				}
			}

			_mb.construct( _size, std::forward<ARGS>(args)... );

			return Accessor<MUTAB>( *this, _size++ );
		}

		Val pop_back() {
			static_assert(Dense || Exists, "can't pop_back() if last element is unknown");

			if constexpr(Exists) {
				while(!_mb.exists(_size-1)) {
					--_size;
					DCHECK_GE(_size, 0);
				}
			}

			DCHECK_GE(_size, 1) << "pop_back() on empty Vector";

			DCHECK(exists(_size-1));

			Val result( std::move(_mb[_size-1]) );
			_mb.destruct(--_size);
			return result;
		}

		auto back()       { return Accessor<MUTAB>(*this, _size-1); }
		auto back() const { return Accessor<CONST>(*this, _size-1); }


		int count() const {
			if constexpr(Dense) return domain();
			else return _mb.count();
		}

		int domain() const {
			return _size;
		}

		int size() const {
			static_assert(Dense, "size() for Sparse_Vector is a bit ambiguous. Use count() or domain() instead");
			return _size;
		}

		int capacity() const {
			return _mb.size();
		}


		//
		// FUN is (int old_key, int new_key) -> void
		//
		// returns count()
		//
		template<class FUN>
		int compact(const FUN& fun = [](int,int){}) {
			static_assert(Exists, "can only compact if have EXISTS flags");
			static_assert(std::is_move_constructible_v<Val>, "compact() requires move constructible Val type");

			int target = 0;
			for(int i=0; i<_mb.size(); ++i) {
				if(_mb.exists(i) && target != i) {

					_mb.construct( target, std::move( _mb[i] ) );
					_mb.destruct( i );

					fun(i, target);
					++target;
				}
			}

			_mb.resize( target ); // calls destructors for moved objects

			_size = target;

			return target;
		}





	public:
		inline auto begin() {
			static_assert(Dense || Exists, "not iterable if don't have EXISTS flags");
			return Iterator<MUTAB>(*this, 0);
		}

		inline auto begin() const {
			static_assert(Dense || Exists, "not iterable if don't have EXISTS flags");
			return Iterator<CONST>(*this, 0);
		}

		inline auto cbegin() const {
			static_assert(Dense || Exists, "not iterable if don't have EXISTS flags");
			return Iterator<CONST>(*this, 0);
		}


		inline auto end() {
			static_assert(Dense || Exists, "not iterable if don't have EXISTS flags");
			return Iterator<MUTAB>(*this, _size);
		}

		inline auto end() const {
			static_assert(Dense || Exists, "not iterable if don't have EXISTS flags");
			return Iterator<CONST>(*this, _size);
		}

		inline auto cend() const {
			static_assert(Dense || Exists, "not iterable if don't have EXISTS flags");
			return Iterator<CONST>(*this, _size);
		}

	};







	struct With_Builder : Vector {
		FORWARDING_CONSTRUCTOR(With_Builder, Vector);

		template<int NEW_STACK_BUFFER>
		using STACK_BUFFER =
			typename Context< Val, Sparse, typename Memory_Block::template STACK_BUFFER<NEW_STACK_BUFFER> > :: With_Builder;

		using SPARSE =
			typename Context< Val, true, Memory_Block > :: With_Builder;

		using EXISTS_INPLACE =
			typename Context< Val, Sparse, typename Memory_Block::EXISTS_INPLACE > :: With_Builder;

		using EXISTS_BITSET =
			typename Context< Val, Sparse, typename Memory_Block::EXISTS_BITSET > :: With_Builder;

		using EXISTS =
			typename Context< Val, Sparse, typename Memory_Block::EXISTS > :: With_Builder;

		using COUNT =
			typename Context< Val, Sparse, typename Memory_Block::COUNT > :: With_Builder;

		using FULL =
			typename Context< Val, true, typename Memory_Block::FULL > :: With_Builder;
	};




}; // struct Context
} // namespace Vector
} // namespace internal






template< class T >
using Vector = typename internal::Vector::Context<
	T,
	false, // SPARSE
	Memory_Block<T>
> :: With_Builder;











}
