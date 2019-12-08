#pragma once

#include "dynamic-array.hpp"

#include "const-flag.hpp"
#include "inplace-storage.hpp"
#include "accessors.hpp"
#include "handles.hpp"

#include "iterable-base.inl"

#include "memory-block.inl"
#include "hash.hpp"

#include "subscript-tags.hpp"


#include "helper-macros-on.inc"


namespace salgo::_::dynamic_array {



using Handle_Int_Type = int;


template<class P>
struct Handle : Int_Handle_Base<Handle<P>, Handle_Int_Type> {
	using BASE = Int_Handle_Base<Handle<P>, Handle_Int_Type>;
	using BASE::BASE;
};

template<class P>
using Handle_Small = Handle<P>;

// same as Handle, but allow creation from `int`
template<class P>
struct Index : Handle<P> {
	using BASE = Handle<P>;
	using BASE::BASE;

	template<class... ARGS>
	Index(ARGS&&... args) : BASE(std::forward<ARGS>(args)...) {} // BASE::BASE(int) is explicit
};




template<class VAL, bool SPARSE, class MEMORY_BLOCK>
struct Params {
	using Val = VAL;
	using Memory_Block = MEMORY_BLOCK;

	using Allocator = typename MEMORY_BLOCK ::Allocator;
	static constexpr int  Stack_Buffer   = MEMORY_BLOCK :: Stack_Buffer;
	static constexpr bool Exists_Inplace = MEMORY_BLOCK :: Has_Exists_Inplace;
	static constexpr bool Exists_Bitset  = MEMORY_BLOCK :: Has_Exists_Bitset;
	static constexpr bool Exists         = MEMORY_BLOCK :: Has_Exists;
	static constexpr bool Count          = MEMORY_BLOCK :: Has_Count;

	static constexpr bool Sparse         = SPARSE;
	static constexpr bool Dense          = !Sparse;

	static constexpr bool Iterable = Dense || Exists;
	
	using Handle       = dynamic_array::Handle<Params>;
	using Handle_Small = dynamic_array::Handle_Small<Params>;
	using Index        = dynamic_array::Index<Params>;
};






//
// accessor
//
template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C,Context<P>> {
	using BASE = Accessor_Base<C,Context<P>>;
	using BASE::BASE;
	friend Dynamic_Array<P>;

public:
	using BASE::operator=;

	template<class... ARGS>
	void construct(ARGS&&... args) {
		static_assert(C == MUTAB, "called construct() on CONST accessor");
		static_assert(P::Sparse);
		CONT._check_bounds( HANDLE );
		CONT._mb( HANDLE ).construct( std::forward<ARGS>(args)... );
	}

	void destruct() {
		static_assert(C == MUTAB, "called destruct() on CONST accessor");
		static_assert(P::Sparse);
		CONT._check_bounds( HANDLE );
		CONT._mb( HANDLE ).destruct();
	}

	void erase() {
		destruct();
	}

	// assumes element is in bounds
	bool constructed() const {
		CONT._check_bounds( HANDLE );
		if constexpr(P::Dense) return true;
		else return CONT._mb( HANDLE ).constructed();
	}

	// same as `constructed()`, but also checks bounds
	// bool exists_SLOW() const {
	// 	if(!CONT._is_in_bounds( HANDLE )) return false;
	// 	return constructed();
	// }

};





//
// store a `reference` to end()
//
template<class P>
struct End_Iterator {};




//
// iterator
//
template<class P, Const_Flag C>
class Iterator : public Iterator_Base<C,Context<P>> {
	using BASE = Iterator_Base<C,Context<P>>;
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
	friend Dynamic_Array<P>;

private:
	friend BASE;

	void _increment() {
		do ++MUT_HANDLE; while( (int)HANDLE != CONT.domain() && !ACC.constructed() );
	}

	void _decrement() {
		do --MUT_HANDLE; while( !ACC.constructed() );
	}

public:
	bool operator!=(End_Iterator<P>) const { return HANDLE != CONT.domain(); }
};




template<class P>
struct Context {
	using Container = Dynamic_Array<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Accessor = dynamic_array::Accessor<P,C>;

	template<Const_Flag C>
	using Iterator = dynamic_array::Iterator<P,C>;
};













template<class P>
class Dynamic_Array : protected P, public Iterable_Base<Dynamic_Array<P>> {
	static_assert(!(P::Dense && P::Count), "no need for COUNT if dynamic_array is DENSE");

public:
	using typename P::Val;
	using typename P::Allocator;
	static constexpr bool Has_Exists_Inplace = P::Exists_Inplace;
	static constexpr bool Has_Exists_Bitset = P::Exists_Bitset;
	static constexpr bool Has_Exists = P::Exists;
	static constexpr bool Has_Count = P::Count;

	using typename P::Handle;
	using typename P::Handle_Small;
	using typename P::Index;

	template<Const_Flag C> using Accessor = dynamic_array::Accessor<P,C>;
	template<Const_Flag C> using Iterator = dynamic_array::Iterator<P,C>;


private:
	friend Accessor<MUTAB>;
	friend Accessor<CONST>;


private:
	typename P::Memory_Block _mb;
	int _size = 0;




public:
	Dynamic_Array() : _mb( P::Memory_Block::Stack_Buffer ) {}

	template<class... ARGS>
	Dynamic_Array(int size, ARGS&&... args) : _mb( std::max(size, P::Memory_Block::Stack_Buffer )), _size(size) {
		for(int i=0; i<size; ++i) {
			_mb(i).construct( args... );
		}
	}

	Dynamic_Array(std::initializer_list<Val>&& l) {
		reserve( l.size() );
		for(auto&& e : l) {
			emplace_back( std::move(e) );
		}
	}

	~Dynamic_Array() {
		static_assert(P::Dense || P::Exists || std::is_trivially_destructible_v<Val>, "no way to know which destructors have to be called");

		if constexpr(P::Dense && !std::is_trivially_destructible_v<Val>) {
			for(int i=0; i<_size; ++i) {
				_mb(i).destruct();
			}
		}
		DCHECK_LE(_size, _mb.domain());
		// for sparse dynamic_arrays, memory block takes care of destruction
	}


	Dynamic_Array(const Dynamic_Array& o) {
		if constexpr(P::Sparse) {
			// same as default
			_mb = o._mb;
			_size = o._size;
		}
		else {
			reserve( o._mb.domain() );
			_size = o._size;
			for(int i=0; i<_size; ++i) {
				_mb(i).construct( o._mb[i] );
			}
		}
	}

	Dynamic_Array(Dynamic_Array&& o) :
			_mb  (std::move(o._mb)),
			_size(std::move(o._size)) {
		o._size = 0;
	}

	Dynamic_Array& operator=(const Dynamic_Array&) = default;
	Dynamic_Array& operator=(Dynamic_Array&& o) {
		this->~Dynamic_Array();
		new(this) Dynamic_Array( std::move(o) );
		return *this;
	}



	template<class... ARGS>
	void resize(int new_size, ARGS&&... args) {
		if constexpr(P::Dense) {
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

	bool  is_empty() const { return _size == 0; }
	bool not_empty() const { return !is_empty(); }


	void reserve(int capacity) {
		if constexpr(P::Dense) {
			_mb.resize( capacity, [this](int i){return i<_size;} );
		}
		else {
			_mb.resize( capacity );
		}
	}




public:
	auto& operator[](Index key) {
		_check_bounds(key);
		return _mb[ key.a ];
	}

	auto& operator[](Index key) const {
		_check_bounds(key);
		return _mb[ key.a ];
	}

	auto& operator[](First_Tag)       { static_assert(P::Dense, "todo: implement for sparse"); return operator[]( Index(0) ); }
	auto& operator[](First_Tag) const { static_assert(P::Dense, "todo: implement for sparse"); return operator[]( Index(0) ); }


	auto& operator[](Last_Tag)       {
		if constexpr(P::Dense) return operator[]( Index(_size-1) );
		else {
			DCHECK_GT(_mb.count(), 0);
			auto it = Iterator<CONST>(this, Index(_size-1));
			if(!it->constructed()) --it;
			return it.value();
		}
	}

	auto& operator[](Last_Tag) const {
		if constexpr(P::Dense) return operator[]( Index(_size-1) );
		else {
			DCHECK_GT(_mb.count(), 0);
			auto it = Iterator<CONST>(this, Index(_size-1));
			if(!it->constructed()) --it;
			return it.value();
		}
	}



public:
	auto operator()(Index key) {
		return Accessor<MUTAB>(this, key);
	}

	auto operator()(Index key) const {
		return Accessor<CONST>(this, key);
	}

	auto operator()(First_Tag)       { static_assert(P::Dense, "todo: implement for sparse"); return operator()( Index(0) ); }
	auto operator()(First_Tag) const { static_assert(P::Dense, "todo: implement for sparse"); return operator()( Index(0) ); }

	auto operator()(Last_Tag)       { static_assert(P::Dense, "todo: implement for sparse"); return operator()( Index(_size-1) ); }
	auto operator()(Last_Tag) const { static_assert(P::Dense, "todo: implement for sparse"); return operator()( Index(_size-1) ); }







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
		// because of dynamic_array realloc:
		static_assert( P::Dense || P::Exists || (std::is_trivially_move_constructible_v<Val> && std::is_trivially_destructible_v<Val>),
			"non-trivially-constructible types require CONSTRUCTED_FLAGS to emplace_back()" );

		static_assert( std::is_move_constructible_v<Val> );

		if(_size == _mb.domain()) {
			if constexpr(P::Dense) {
				_mb.resize( (_mb.domain() + 1) * 3/2, [](int){ return /*i<_size*/true; } );
			}
			else {
				_mb.resize( (_mb.domain() + 1) * 3/2 );
			}
		}

		_mb(_size).construct( std::forward<ARGS>(args)... );

		return Accessor<MUTAB>( this, Index(_size++) );
	}

	Accessor<MUTAB> push_back(const Val& val) {
		return emplace_back(val);
	}

	template<class... ARGS>
	auto add(ARGS&&... args) {
		return emplace_back( std::forward<ARGS>(args)... );
	}

	auto pop_back() {
		static_assert(P::Dense || P::Exists, "can't pop_back() if last element is unknown");

		if constexpr(P::Exists) {
			while(!_mb(_size-1).constructed()) {
				--_size;
				DCHECK_GE(_size, 0);
			}
		}

		DCHECK_GE(_size, 1) << "pop_back() on empty Dynamic_Array";

		DCHECK( (*this)(_size-1).constructed() );

		Val result( std::move(_mb[_size-1]) );
		_mb(--_size).destruct();
		return result;
	}



	int count() const {
		if constexpr(P::Dense) return domain();
		else return _mb.count();
	}

	int domain() const {
		return _size;
	}

	int size() const {
		static_assert(P::Dense, "size() for Sparse_Dynamic_Array is a bit ambiguous. Use count() or domain() instead");
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
		static_assert(P::Exists, "can only compact if have CONSTRUCTED_FLAGS flags");
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
		static_assert(P::Iterable);
		auto e = Iterator<MUTAB>(this, Index(0));
		if(_size && !e->constructed()) ++e;
		return e;
	}

	auto begin() const {
		static_assert(P::Iterable);
		auto e = Iterator<CONST>(this, Index(0));
		if(_size && !e->constructed()) ++e;
		return e;
	}



	auto end() const {
		static_assert(P::Iterable);
		return End_Iterator<P>();
	}

};






template<class P>
class With_Builder : public Dynamic_Array<P> {
	using BASE = Dynamic_Array<P>;

public:
	//FORWARDING_CONSTRUCTOR(With_Builder, BASE) {}
	//FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(With_Builder, BASE) {}
	using BASE::BASE;

	using typename P::Val;
	using P::Sparse;
	using typename P::Memory_Block;

	template<int X>
	using INPLACE_BUFFER = With_Builder< Params<
		Val, Sparse, typename Memory_Block::template INPLACE_BUFFER<X> >>;

	using CONSTRUCTED_FLAGS = With_Builder< Params<
		Val, Sparse, typename Memory_Block::CONSTRUCTED_FLAGS >>;

	using CONSTRUCTED_FLAGS_INPLACE = With_Builder< Params<
		Val, Sparse, typename Memory_Block::CONSTRUCTED_FLAGS_INPLACE >>;

	using CONSTRUCTED_FLAGS_BITSET = With_Builder< Params<
		Val, Sparse, typename Memory_Block::CONSTRUCTED_FLAGS_BITSET >>;


	// also enable CONSTRUCTED_FLAGS by default
	using SPARSE = With_Builder< Params<
		Val, true, Memory_Block >>;


	// just enable SPARSE, but no CONSTRUCTED_FLAGS
	using SPARSE_NO_CONSTRUCTED_FLAGS = With_Builder< Params<
		Val, true, Memory_Block >>;



	using COUNT = With_Builder< Params<
		Val, Sparse, typename Memory_Block::COUNT >>;


	using FULL_BLOWN = With_Builder< Params<
		Val, true, typename Memory_Block::FULL_BLOWN >>;
};




} // namespace salgo::_::Dynamic_Array

#include "helper-macros-off.inc"





namespace salgo {

template<class P>
struct Hash<_::dynamic_array::Handle<P>> {
	auto operator()(const _::dynamic_array::Handle<P>& h) const { return h; }
};

} // namespace salgo