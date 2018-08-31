#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "stack-storage.hpp"
#include "accessors.hpp"
#include "handles.hpp"

#include "memory-block.hpp"


#include "helper-macros-on"


namespace salgo {






namespace internal {
namespace Vector {




template<class _VAL, bool _SPARSE, class _MEMORY_BLOCK>
struct Context {

	//
	// forward declarations
	//
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Vector;

	using Container = Vector;



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

	static constexpr bool Iterable = Dense || Exists;




	struct Handle : Int_Handle_Base<Handle,int> {
		using BASE = Int_Handle_Base<Handle,int>;
		EXPLICIT_FORWARDING_CONSTRUCTOR(Handle, BASE) {}
	};

	using Handle_Small = Handle;

	// same as Handle, but allow creation from `int`
	struct Index : Handle {
		FORWARDING_CONSTRUCTOR(Index, Handle) {}
	};








	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		friend Vector;

		using BASE::container;
		using BASE::_handle;

	public:
		using BASE::operator=;

		template<class... ARGS>
		void construct(ARGS&&... args) {
			static_assert(C == MUTAB, "called construct() on CONST accessor");
			static_assert(Sparse);
			CONT._check_bounds( _handle() );
			CONT._mb( _handle() ).construct( std::forward<ARGS>(args)... );
		}

		void destruct() {
			static_assert(C == MUTAB, "called destruct() on CONST accessor");
			static_assert(Sparse);
			CONT._check_bounds( _handle() );
			CONT._mb( _handle() ).destruct();
		}

		void erase() {
			destruct();
		}

		// assumes element is in bounds
		bool constructed() const {
			CONT._check_bounds( _handle() );
			if constexpr(Dense) return true;
			else return CONT._mb( _handle() ).constructed();
		}

		// same as `constructed()`, but also checks bounds
		bool exists_SLOW() const {
			if(!CONT._is_in_bounds( _handle() )) return false;
			return constructed();
		}

	};





	//
	// store a `reference` to end()
	//
	struct End_Iterator {};




	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
		friend Vector;

		using BASE::container;
		using BASE::_handle;

	private:
		friend Iterator_Base<C,Context>;

		void _increment() {
			do ++_handle(); while( (int)_handle() != CONT.domain() && !CONT( _handle() ).constructed() );
		}

		void _decrement() {
			do --_handle(); while( !CONT( _handle() ).constructed() );
		}

	public:
		bool operator!=(End_Iterator) { return HANDLE < CONT.domain(); }
	};

















	class Vector {
		static_assert(!(Dense && Count), "no need for COUNT if vector is DENSE");

	public:
		using Val = Context::Val;
		static constexpr bool Has_Exists_Inplace = Context::Exists_Inplace;
		static constexpr bool Has_Exists_Bitset = Context::Exists_Bitset;
		static constexpr bool Has_Exists = Context::Exists;
		static constexpr bool Has_Count = Context::Count;

		using Handle       = Context::Handle;
		using Handle_Small = Context::Handle_Small;
		using Index        = Context::Index;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;


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
		Vector() : _mb( Memory_Block::Stack_Buffer ) {}

		template<class... ARGS>
		Vector(int size, ARGS&&... args) : _mb( std::max(size, Memory_Block::Stack_Buffer )), _size(size) {
			for(int i=0; i<size; ++i) {
				_mb(i).construct( args... );
			}
		}

		Vector(std::initializer_list<Val>&& l) {
			reserve( l.size() );
			for(auto&& e : l) {
				emplace_back( std::move(e) );
			}
		}

		~Vector() {
			static_assert(!(Sparse && !Exists && !std::is_trivially_move_constructible_v<Val>),
				"no way to know which destructors have to be called");

			if constexpr(Dense) {
				for(int i=0; i<_size; ++i) {
					_mb(i).destruct();
				}
			}
			DCHECK_LE(_size, _mb.domain());
			// for sparse vectors, memory block takes care of destruction
		}


		Vector(const Vector&) = default;
		Vector(Vector&& o) {
			_mb = std::move(o._mb);
			_size = o._size;
			o._size = 0;
		}

		Vector& operator=(const Vector&) = default;
		Vector& operator=(Vector&& o) {
			this->~Vector();
			new(this) Vector( std::move(o) );
			return *this;
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
			for(int i=_size; i<_mb.domain(); ++i) {
				_mb(i).construct( args... );
			}

			_size = new_size;
		}

		void clear() { resize(0); }
		bool empty() const { return _size == 0; }


		void reserve(int capacity) {
			if constexpr(Dense) {
				_mb.resize( capacity, [this](int i){return i<_size;} );
			}
			else {
				_mb.resize( capacity );
			}
		}




		// direct access
	public:
		auto& operator[](Index key) {
			_check_bounds(key);
			return _mb[ key.a ];
		}

		auto& operator[](Index key) const {
			_check_bounds(key);
			return _mb[ key.a ];
		}

		auto& operator[](First_Tag)       { static_assert(Dense, "todo: implement for sparse"); return operator[]( Index(0) ); }
		auto& operator[](First_Tag) const { static_assert(Dense, "todo: implement for sparse"); return operator[]( Index(0) ); }

		auto& operator[](Last_Tag)       { static_assert(Dense, "todo: implement for sparse"); return operator[]( Index(_size-1) ); }
		auto& operator[](Last_Tag) const { static_assert(Dense, "todo: implement for sparse"); return operator[]( Index(_size-1) ); }



		// accessor access
	public:
		auto operator()(Index key) {
			_check_bounds(key);
			return Accessor<MUTAB>(this, key);
		}

		auto operator()(Index key) const {
			_check_bounds(key);
			return Accessor<CONST>(this, key);
		}

		auto operator()(First_Tag)       { static_assert(Dense, "todo: implement for sparse"); return operator()( Index(0) ); }
		auto operator()(First_Tag) const { static_assert(Dense, "todo: implement for sparse"); return operator()( Index(0) ); }

		auto operator()(Last_Tag)       { static_assert(Dense, "todo: implement for sparse"); return operator()( Index(_size-1) ); }
		auto operator()(Last_Tag) const { static_assert(Dense, "todo: implement for sparse"); return operator()( Index(_size-1) ); }







	private:
		bool _is_in_bounds(Index key) const {
			return key >= 0 && key < domain();
		}

		void _check_bounds(Index key) const {
			DCHECK(_is_in_bounds(key)) << "index " << key << " out of bounds [0," << domain() << ")";
		}





	public:
		template<class... ARGS>
		Accessor<MUTAB> emplace_back(ARGS&&... args) {
			// because of vector realloc:
			static_assert( Dense || Exists || std::is_trivially_move_constructible_v<Val>,
				"non-trivially-constructible types require CONSTRUCTED_FLAGS to emplace_back()" );

			if(_size == _mb.domain()) {
				if constexpr(Dense) {
					_mb.resize( (_mb.domain() + 1) * 3/2, [](int){ return /*i<_size*/true; } );
				}
				else {
					_mb.resize( (_mb.domain() + 1) * 3/2 );
				}
			}

			_mb(_size).construct( std::forward<ARGS>(args)... );

			return Accessor<MUTAB>( this, Index(_size++) );
		}

		template<class... ARGS>
		auto add(ARGS&&... args) {
			return emplace_back( std::forward<ARGS>(args)... );
		}

		auto pop_back() {
			static_assert(Dense || Exists, "can't pop_back() if last element is unknown");

			if constexpr(Exists) {
				while(!_mb(_size-1).constructed()) {
					--_size;
					DCHECK_GE(_size, 0);
				}
			}

			DCHECK_GE(_size, 1) << "pop_back() on empty Vector";

			DCHECK( (*this)(_size-1).constructed() );

			Val result( std::move(_mb[_size-1]) );
			_mb(--_size).destruct();
			return result;
		}



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
			return _mb.domain();
		}


		//
		// FUN is (int old_handle, int new_handle) -> void
		//
		// returns count()
		//
		template<class FUN>
		int compact(const FUN& fun = [](int,int){}) {
			static_assert(Exists, "can only compact if have CONSTRUCTED_FLAGS flags");
			static_assert(std::is_move_constructible_v<Val>, "compact() requires move constructible Val type");

			int target = 0;
			for(int i=0; i<_mb.domain(); ++i) {
				if(_mb(i).constructed() && target != i) {

					_mb(target).construct( std::move( _mb[i] ) );
					_mb(i).destruct();

					fun(i, target);
					++target;
				}
			}

			_mb.resize( target ); // calls destructors for moved objects

			_size = target;

			return target;
		}





	public:
		auto begin() {
			static_assert(Iterable);
			auto e = Iterator<MUTAB>(this, Index(0));
			if(_size && !e->constructed()) ++e;
			return e;
		}

		auto begin() const {
			static_assert(Iterable);
			auto e = Iterator<CONST>(this, Index(0));
			if(_size && !e->constructed()) ++e;
			return e;
		}



		auto end() const {
			static_assert(Iterable);
			return End_Iterator();
		}

	};







	struct With_Builder : Vector {
		FORWARDING_CONSTRUCTOR(With_Builder, Vector) {}
		FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(With_Builder, Vector) {}

		template<int X>
		using INPLACE_BUFFER =
			typename Context< Val, Sparse, typename Memory_Block::template INPLACE_BUFFER<X> > :: With_Builder;


		using CONSTRUCTED_FLAGS =
			typename Context< Val, Sparse, typename Memory_Block::CONSTRUCTED_FLAGS > :: With_Builder;

		using CONSTRUCTED_FLAGS_INPLACE =
			typename Context< Val, Sparse, typename Memory_Block::CONSTRUCTED_FLAGS_INPLACE > :: With_Builder;

		using CONSTRUCTED_FLAGS_BITSET =
			typename Context< Val, Sparse, typename Memory_Block::CONSTRUCTED_FLAGS_BITSET > :: With_Builder;


		// also enable CONSTRUCTED_FLAGS by default
		using SPARSE =
			typename Context< Val, true, Memory_Block > :: With_Builder :: CONSTRUCTED_FLAGS;


		// just enable SPARSE, but no CONSTRUCTED_FLAGS
		using SPARSE_NO_CONSTRUCTED_FLAGS =
			typename Context< Val, true, Memory_Block > :: With_Builder;



		using COUNT =
			typename Context< Val, Sparse, typename Memory_Block::COUNT > :: With_Builder;


		using FULL_BLOWN =
			typename Context< Val, true, typename Memory_Block::FULL_BLOWN > :: With_Builder;
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











} // namespace salgo

#include "helper-macros-off"
