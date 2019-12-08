#pragma once

#include "memory-block.hpp"

#include "handles.hpp"
#include "accessors.hpp"
#include "inplace-storage.hpp"
#include "subscript-tags.hpp"
#include "const-flag.hpp"
#include "add-member.hpp"

#include <cstring> // memcpy

#include "helper-macros-on.inc"

namespace salgo::_::memory_block {

SALGO_ADD_MEMBER(num_existing)
SALGO_ADD_MEMBER(exists)


			template<bool> struct Add_exists_bitset {
				std::vector<bool> exists; // TODO: don't store size and capacity here
			};
			template<> struct Add_exists_bitset<false> {};


using Handle_Int_Type = int;

template<class P>
struct Handle : Int_Handle_Base<Handle<P>, Handle_Int_Type> {
	using BASE = Int_Handle_Base<Handle<P>, Handle_Int_Type>;
	using BASE::BASE;
};


// same as Handle, but allow creation from `int`
template<class P>
struct Index : Handle<P> {
	using BASE = Handle<P>;
	using BASE::BASE;

	FORWARDING_CONSTRUCTOR(Index, BASE) {} // makes base constructors non-explicit (construction from int)
	//Index(int i) : BASE(i) {} // BASE::BASE(int) is explicit
};



template<class VAL, class ALLOCATOR, int STACK_BUFFER, bool DENSE,
		bool CONSTRUCTED_FLAGS_INPLACE, bool CONSTRUCTED_FLAGS_BITSET, bool COUNT, int ALIGN>
struct Params {
	using Val = VAL;
	using Supplied_Allocator = ALLOCATOR;

	static constexpr int  Stack_Buffer = STACK_BUFFER;
	static constexpr bool Dense = DENSE;
	static constexpr bool Sparse = !Dense;
	static constexpr bool Exists_Inplace = CONSTRUCTED_FLAGS_INPLACE;
	static constexpr bool Exists_Bitset = CONSTRUCTED_FLAGS_BITSET;
	static constexpr bool Exists = CONSTRUCTED_FLAGS_BITSET || CONSTRUCTED_FLAGS_INPLACE;
	static constexpr bool Count = COUNT;


	static constexpr bool Iterable = Exists || Dense;
	static constexpr bool Countable = Count || Dense;



	using Handle       = memory_block::Handle<Params>;
	using Handle_Small = Handle;
	using Index        = memory_block::Index<Params>;

	static constexpr auto Align = ALIGN;
	struct alignas(Align) Node : salgo::Inplace_Storage<Val>, Add_exists<bool, Exists_Inplace> {};

	using Rebound_Allocator = typename std::allocator_traits< ALLOCATOR >::template rebind_alloc<Node>;
};




template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C,Context<P>> {
	using BASE = Accessor_Base<C,Context<P>>;

public:
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

public:
	auto index() const { return BASE::handle(); }



	template<class... ARGS>
	auto& construct(ARGS&&... args) {
		static_assert(C == MUTAB, "called construct() on CONST accessor");
		static_assert(!P::Dense, "construct() not supported for DENSE memory-blocks");

		_check_bounds();
		if constexpr(P::Exists) {
			DCHECK( !constructed() ) << "element already constructed";
			CONT._set_exists(HANDLE, true);
		}
		_get().construct( std::forward<ARGS>(args)... );
		if constexpr(P::Count) ++CONT.num_existing;

		return *this;
	}

	auto& destruct() {
		static_assert(C == MUTAB, "called destruct() on CONST accessor");
		static_assert(!P::Dense, "destruct() not supported for DENSE memory-blocks");

		_check_bounds();
		if constexpr(P::Exists) {
			DCHECK( constructed() ) << "erasing already erased element";
			CONT._set_exists(HANDLE, false);
		}
		_get().destruct();
		if constexpr(P::Count) --CONT.num_existing;

		return *this;
	}


	// assumes key is in bounds
	bool constructed() const {
		_check_bounds();
		static_assert(P::Dense || P::Exists, "called constructed() on object without CONSTRUCTED_FLAGS or DENSE");

		if constexpr(P::Dense) {
			return true;
		} else if constexpr(P::Exists_Inplace) {
			return _get().exists;
		} else if constexpr(P::Exists_Bitset) {
			return CONT.exists[ HANDLE ];
		}
	}

	// same as `constructed()` but also checks bounds
	// bool exists_SLOW() const {
	// 	if(!_is_in_bounds()) return false;
	// 	return constructed();
	// }

private:
	bool _is_in_bounds() const {
		return CONT._is_in_bounds( HANDLE );
	}

	void _check_bounds() const {
		CONT._check_bounds( HANDLE );
	}

	auto& _get()       { return CONT._get( HANDLE ); }
	auto& _get() const { return CONT._get( HANDLE ); }
};




template<class P>
class End_Iterator {};




template<class P, Const_Flag C>
class Iterator : public Iterator_Base<C,Context<P>> {
	using BASE = Iterator_Base<C,Context<P>>;

public:
	using BASE::BASE;

private:
	friend BASE;

	void _increment() {
		if constexpr(P::Dense) ++MUT_HANDLE;
		else do ++MUT_HANDLE; while( (int)HANDLE != CONT.domain() && !BASE::accessor().constructed() );
	}

	void _decrement() {
		if constexpr(P::Dense) --MUT_HANDLE;
		else do --MUT_HANDLE; while( !BASE::accessor().constructed() );
	}

public:
	bool operator!=(const End_Iterator<P>&) { return HANDLE != CONT.domain(); }
};







template<class P>
struct Context {
	using Container = Memory_Block<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Accessor = memory_block::Accessor<P,C>;

	template<Const_Flag C>
	using Iterator = memory_block::Iterator<P,C>;
};








template<class P>
class Memory_Block :
		private P::Rebound_Allocator,
		private Add_num_existing<int, P::Count>,
		//private Add_exists<std::vector<bool>, P::Exists_Bitset>,
		private Add_exists_bitset<P::Exists_Bitset>,
		protected P {

	using NUM_EXISTING_BASE = Add_num_existing<int, P::Count>;
	using CONSTRUCTED_FLAGS_BITSET_BASE = Add_exists_bitset<P::Exists_Bitset>;//Add_exists<std::vector<bool>, P::Exists_Bitset>;

	using typename P::Node;

public:
	using typename P::Val;
	using Allocator = typename P::Rebound_Allocator;
	using P::Stack_Buffer;
	static constexpr bool Is_Dense = P::Dense;
	static constexpr bool Is_Sparse = P::Sparse;
	static constexpr bool Has_Exists_Inplace = P::Exists_Inplace;
	static constexpr bool Has_Exists_Bitset = P::Exists_Bitset;
	static constexpr bool Has_Exists = P::Exists;
	static constexpr bool Has_Count = P::Count;

	using Handle       = typename P::Handle;
	using Handle_Small = typename P::Handle_Small;
	using Index        = typename P::Index;

private:
	friend Accessor<P,MUTAB>;
	friend Accessor<P,CONST>;


	static_assert(!(P::Dense && P::Count), "no need for COUNT if vector is DENSE");

private:
	// avoid instantiating Node if not needed
	static constexpr int _stack_buffer_sizeof() {
		if constexpr(Stack_Buffer > 0) {
			return Stack_Buffer * sizeof(Node);
		} else return 0;
	}

	//
	// data
	//
private:
	Node* _data = _get_stack_buffer();
	char _stack_buffer[ _stack_buffer_sizeof() ];

	int _size = 0; // not Stack_Buffer? if so, need to also construct elements if DENSE

	auto _get_stack_buffer() {
		return (Node*)_stack_buffer;
	}

	auto _get_stack_buffer() const {
		return (const Node*)_stack_buffer;
	}

	auto& _get(Index key)       { return _data[key]; }
	auto& _get(Index key) const { return _data[key]; }


public:
	// copy-construct
	Memory_Block(const Memory_Block& o) :
			Allocator(o),
			NUM_EXISTING_BASE(o),
			CONSTRUCTED_FLAGS_BITSET_BASE(o),
			_size(o._size) {

		static_assert(P::Dense || P::Exists || std::is_trivially_copy_constructible_v<Val>,
						"can't copy-construct non-POD container if no CONSTRUCTED_FLAGS or DENSE flags");

		if(_size > Stack_Buffer) {
			_data = std::allocator_traits<Allocator>::allocate(_allocator(), _size);
		}
		else {
			_data = _get_stack_buffer();
		}

		for(int i=0; i<_size; ++i) {
			std::allocator_traits<Allocator>::construct(_allocator(), _data+i);
			if( o(i).constructed() ) {
				_get(i).construct( o[i] );
			}
		}
	}

	// move-construct
	Memory_Block(Memory_Block&& o) :
			Allocator( std::move(o) ),
			NUM_EXISTING_BASE( std::move(o) ),
			CONSTRUCTED_FLAGS_BITSET_BASE( std::move(o) ),
			_size( std::move(o._size) ) {

		if(_size > Stack_Buffer) {
			_data = o._data;
		}
		else if constexpr( _stack_buffer_sizeof() > 0 ) {
			_data = _get_stack_buffer();
			std::memcpy( _stack_buffer, o._stack_buffer, _stack_buffer_sizeof() );
		}

		o._size = 0;
	}

	// copy assignment
	Memory_Block& operator=(const Memory_Block& o) {
		this->~Memory_Block();
		new(this) Memory_Block(o);
		return *this;
	}

	// move assignment
	Memory_Block& operator=(Memory_Block&& o) {
		this->~Memory_Block();
		new(this) Memory_Block( std::move(o) );
		return *this;
	}


	//
	// construction
	//
public:
	Memory_Block() {
		static_assert(!P::Exists_Inplace || !P::Exists_Bitset, "can't have both");
		static_assert(!P::Dense || !P::Exists, "can't have both");
		static_assert(!P::Dense || !P::Count, "can't have both");
	}

	template<class... ARGS>
	Memory_Block(int size, ARGS&&... args) : _size(size) {
		static_assert(P::Dense || sizeof...(ARGS) == 0, "only DENSE memory_blocks can supply construction args");

		if(_size > Stack_Buffer) {
			_data = std::allocator_traits<Allocator>::allocate(_allocator(), _size);
			//std::cout << "allocated " << _size << " elements of sizeof " << sizeof()
		}
		else {
			_data = _get_stack_buffer();
		}

		for(int i=0; i<_size; ++i) {
			std::allocator_traits<Allocator>::construct(_allocator(), _data+i);
			if constexpr(P::Dense) {
				_get(i).construct( args... );
			}
		}

		if constexpr(P::Exists_Bitset) CONSTRUCTED_FLAGS_BITSET_BASE::exists.resize( _size );
	}

	~Memory_Block() {
		if constexpr(P::Dense || P::Exists) {
			_destruct_block(_data, _size);
		}

		// remove heap block
		if(_size > Stack_Buffer) {
			std::allocator_traits<Allocator>::deallocate(_allocator(), _data, _size);
		}
	}

private:
	auto& _allocator()       { return *static_cast<      Allocator*>(this); }
	auto& _allocator() const { return *static_cast<const Allocator*>(this); }


private:
	void _destruct_block(Node* data, int size) {
		static_assert(P::Dense || P::Exists || std::is_trivially_destructible_v<Val>,
						"can't destroy non-POD container if no CONSTRUCTED_FLAGS or DENSE flags");

		_destruct_block(data, size, [this](int i){ return (*this)(i).constructed(); });
	}

	// destruct nodes+values
	template<class CONSTRUCTED_FLAGS_FUN>
	void _destruct_block(Node* data, int size, CONSTRUCTED_FLAGS_FUN&& exists_fun) {
		(void)data;
		if constexpr(!std::is_trivially_destructible_v<Val>) {

			// destruct values
			for(int i=0; i<size; ++i) if(exists_fun(i)) {
					_get(i).destruct();
				}
		}

		// destruct nodes
		for(int i=0; i<size; ++i) {
			std::allocator_traits<Allocator>::destroy(_allocator(), data+i);
		}
	}

public:
	void resize(int new_size) {
		static_assert(P::Dense || P::Exists || std::is_trivially_move_constructible_v<Val>,
						"can't resize non-POD container if no CONSTRUCTED_FLAGS or DENSE flags");

		if constexpr(std::is_trivially_move_constructible_v<Val>) {
			_resize(new_size, [](int){ return true; });
		}
		else _resize(new_size, [this](int i){ return (*this)(i).constructed(); });
	}

	template<class CONSTRUCTED_FLAGS_FUN>
	void resize(int new_size, CONSTRUCTED_FLAGS_FUN&& exists_fun) {
		static_assert(!P::Dense && !P::Exists, "can only supply CONSTRUCTED_FLAGS_FUN if not Dense and Exists");

		_resize(new_size, std::forward<CONSTRUCTED_FLAGS_FUN>(exists_fun));
	}

private:
	template<class CONSTRUCTED_FLAGS_FUN>
	void _resize(int new_size, CONSTRUCTED_FLAGS_FUN&& exists_fun) {

		decltype(_data) new_data;

		if(new_size > Stack_Buffer) {
			new_data = std::allocator_traits<Allocator>::allocate(_allocator(), new_size);
		}
		else {
			new_data = _get_stack_buffer();
		}

		int n = std::min(_size, new_size);

		// new memory location?
		if(new_data != _data) {

			if constexpr(!std::is_trivially_move_constructible_v<Val>) {
				for(int i=0; i<n; ++i) {
					if(exists_fun(i)) {
						// move-construct node+value

						// std::cout << "move construct" << std::endl;
						// auto _align = [](auto ptr){
						// 	return uint64_t(ptr) % 64;
						// };
						// std::cout << "from align " << _align(&_data[i]) << std::endl;
						// std::cout << "to align " << _align(new_data+i) << std::endl;

						std::allocator_traits<Allocator>::construct(_allocator(), new_data+i, std::move( _data[i] ));

						// std::cout << "move construct done" << std::endl;
					}
					else {
						// construct empty node
						std::allocator_traits<Allocator>::construct(_allocator(), new_data+i);
					}
				}
			}
			else {
				static_assert(std::is_trivially_move_constructible_v<Node>);
				std::memcpy(new_data, _data, n * sizeof(Node));
			}

			// std::cout << "destroy old block" << std::endl;
			_destruct_block(_data, _size, std::forward<CONSTRUCTED_FLAGS_FUN>(exists_fun));
			// std::cout << "destroy old block done" << std::endl;
		}
		else {
			// same memory location
			_destruct_block(_data + n, _size - n, std::forward<CONSTRUCTED_FLAGS_FUN>(exists_fun));
		}

		// construct new nodes
		for(int i=_size; i<new_size; ++i) {
			std::allocator_traits<Allocator>::construct(_allocator(), new_data+i);
			if constexpr(P::Dense) {
				new_data[i].construct(); // todo: supply args
			}
		}


		// remove old block
		if(_size > Stack_Buffer) {
			std::allocator_traits<Allocator>::deallocate(_allocator(), _data, _size);
		}

		_data = new_data;
		_size = new_size;

		if constexpr(P::Exists_Bitset) CONSTRUCTED_FLAGS_BITSET_BASE::exists.resize( new_size );
	}


public:
	auto domain() const { return _size; }

	auto size() const {
		static_assert(P::Dense, "size() is ambiguous when memory block is Sparse");
		return _size;
	}




	// direct access
public:
	Val& operator[](Index key) {
		_check_bounds(key);
		if constexpr(P::Exists) DCHECK( (*this)(key).constructed() ) << "accessing non-constructed element with key " << key;
		return _get(key).get();
	}

	const Val& operator[](Index key) const {
		_check_bounds(key);
		if constexpr(P::Exists) DCHECK( (*this)(key).constructed() ) << "accessing non-constructed element with key " << key;
		return _get(key).get();
	}


	auto& operator[](First_Tag)       { return operator()(FIRST)(); }
	auto& operator[](First_Tag) const { return operator()(FIRST)(); }

	auto& operator[](Last_Tag)        { return operator()(LAST)(); }
	auto& operator[](Last_Tag)  const { return operator()(LAST)(); }




	// accessor access
public:
	auto operator()(Index key) {
		_check_bounds(key);
		return Accessor<P,MUTAB>(this, key);
	}

	auto operator()(Index key) const {
		_check_bounds(key);
		return Accessor<P,CONST>(this, key);
	}


	auto operator()(First_Tag) {
		DCHECK(!empty());
		Handle h = Index(0);
		while(!(*this)(h).constructed()) ++h;
		return operator()(h);
	}

	auto operator()(First_Tag) const {
		DCHECK(!empty());
		Handle h = Index(0);
		while(!(*this)(h).constructed()) ++h;
		return operator()(h);
	}


	auto operator()(Last_Tag) {
		DCHECK(!empty());
		Handle h = Index( domain() );
		do --h; while(!(*this)(h).constructed());
		return operator()(h);
	}

	auto operator()(Last_Tag) const {
		DCHECK(!empty());
		Handle h = Index( domain() );
		do --h; while(!(*this)(h).constructed());
		return operator()(h);
	}





public:
	template<class... ARGS>
	void construct_all(const ARGS&... args) {
		static_assert(!P::Dense, "construct_all() not supported for DENSE memory-blocks");

		for(int i=0; i<_size; ++i) {
			if constexpr(P::Exists) {
				DCHECK( !(*this)(i).constructed() ) << "can't construct_all() if some elements already exist";
			}
			(*this)(i).construct( args... );
		}
	}


private:
	bool _is_in_bounds(Index key) const {
		return key >= 0 && key < domain();
	}

	void _check_bounds(Index key) const {
		DCHECK( _is_in_bounds(key) ) << "index " << key << " out of bounds [0," << domain() << ")";
	}



public:
	int count() const {
		static_assert(P::Countable);
		if constexpr(P::Dense) return domain();
		else return NUM_EXISTING_BASE::num_existing;
	}

	bool empty() const {
		static_assert(P::Countable);
		return count() == 0;
	}




public:
	auto begin() {
		static_assert(P::Iterable);
		auto e = Iterator<P,MUTAB>(this, Index(0));
		if(_size && !e.accessor().constructed()) ++e;
		return e;
	}

	auto begin() const {
		static_assert(P::Iterable);
		auto e = Iterator<P,CONST>(this, Index(0));
		if(_size && !e.accessor().constructed()) ++e;
		return e;
	}

	auto end() const {
		static_assert(P::Iterable);
		return End_Iterator<P>();
	}




private:
	void _set_exists(Handle key, bool new_exists) {
		static_assert(P::Exists);

		if constexpr(P::Exists_Inplace) {
			_get(key).exists = new_exists;
		}
		else if constexpr(P::Exists_Bitset) {
			CONSTRUCTED_FLAGS_BITSET_BASE::exists[key] = new_exists;
		}
	}

};






template<class P>
class With_Builder : public Memory_Block<P> {
	using BASE = Memory_Block<P>;

public:
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(With_Builder, Memory_Block) {}

	using typename P::Val;
	using typename P::Supplied_Allocator;
	using P::Stack_Buffer;
	using P::Dense;
	using P::Exists_Inplace;
	using P::Exists_Bitset;
	using P::Count;
	using P::Align;


	template<class NEW_ALLOCATOR>
	using ALLOCATOR =
		With_Builder< Params< Val, NEW_ALLOCATOR, Stack_Buffer, Dense, Exists_Inplace, Exists_Bitset, Count, Align >>;

	template<int NEW_STACK_BUFFER>
	using INPLACE_BUFFER =
		With_Builder< Params< Val, Supplied_Allocator, NEW_STACK_BUFFER, Dense, Exists_Inplace, Exists_Bitset, Count, Align >>;


	using DENSE =
		With_Builder< Params< Val, Supplied_Allocator, Stack_Buffer, true, Exists_Inplace, Exists_Bitset, Count, Align >>;

	using SPARSE = // (default)
		With_Builder< Params< Val, Supplied_Allocator, Stack_Buffer, false, Exists_Inplace, Exists_Bitset, Count, Align >>;


	using CONSTRUCTED_FLAGS_INPLACE =
		With_Builder< Params< Val, Supplied_Allocator, Stack_Buffer, Dense, true, false, Count, Align >>;

	using CONSTRUCTED_FLAGS_BITSET =
		With_Builder< Params< Val, Supplied_Allocator, Stack_Buffer, Dense, false, true, Count, Align >>;

	using CONSTRUCTED_FLAGS = CONSTRUCTED_FLAGS_BITSET; // seems to be faster than inplace version - also much more memory efficient for large aligned types

	using COUNT =
		With_Builder< Params< Val, Supplied_Allocator, Stack_Buffer, Dense, Exists_Inplace, Exists_Bitset, true, Align >>;

	template<int X>
	using ALIGN =
		With_Builder< Params< Val, Supplied_Allocator, Stack_Buffer, Dense, Exists_Inplace, Exists_Bitset, Count, X >>;

	using FULL_BLOWN =
		With_Builder< Params< Val, Supplied_Allocator, Stack_Buffer, Dense, false, true, true, Align >>; // by default bitset-exists
};


} // namespace salgo::_::Memory_Block

#include "helper-macros-off.inc"





template<class X>
struct std::hash<salgo::_::memory_block::Handle<X>> {
	size_t operator()(const salgo::_::memory_block::Handle<X>& h) const {
		return std::hash<salgo::_::memory_block::Handle_Int_Type>()(
			salgo::_::memory_block::Handle_Int_Type(h)
		);
	}
};

