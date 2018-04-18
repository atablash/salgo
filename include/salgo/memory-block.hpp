#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "stack-storage.hpp"
#include "accessors.hpp"
#include "int-handle.hpp"

#include <cstring> // memcpy


namespace salgo {






namespace internal {
namespace Memory_Block {


template<bool> struct Add_num_existing { int num_existing = 0; };
template<> struct Add_num_existing<false> {};


template<bool> struct Add_exists { bool exists = false; };
template<> struct Add_exists<false> {};


template<bool> struct Add_exists_bitset {
	std::vector<bool> exists; // TODO: don't store size and capacity here
};
template<> struct Add_exists_bitset<false> {};



template<class _VAL, class _ALLOCATOR, int _STACK_BUFFER, bool _DENSE,
	bool _EXISTS_INPLACE, bool _EXISTS_BITSET, bool _COUNT>
struct Context {

	//
	// forward declarations
	//
	struct Node;
	template<Const_Flag C> class Accessor;
	class Memory_Block;
	using Container = Memory_Block;





	using Val = _VAL;
	using Allocator = typename std::allocator_traits< _ALLOCATOR >::template rebind_alloc<Node>;
	static constexpr int  Stack_Buffer = _STACK_BUFFER;
	static constexpr bool Dense = _DENSE;
	static constexpr bool Sparse = !Dense;
	static constexpr bool Exists_Inplace = _EXISTS_INPLACE;
	static constexpr bool Exists_Bitset = _EXISTS_BITSET;
	static constexpr bool Exists = _EXISTS_BITSET || _EXISTS_INPLACE;
	static constexpr bool Count = _COUNT;



	static constexpr bool Iterable = Exists || Dense;
	static constexpr bool Countable = Count || Dense;






	struct Handle : Int_Handle_Base<Handle,int> {
		using BASE = Int_Handle_Base<Handle,int>;
		FORWARDING_CONSTRUCTOR( Handle, BASE ) {}
	};



	struct Node : salgo::Stack_Storage<Val>, Add_exists<Exists_Inplace> {};






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

	public:
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

	public:
		auto index() const { return BASE::handle(); }


		template<class... ARGS>
		void construct(ARGS&&... args) {
			static_assert(C == MUTAB, "called construct() on CONST accessor");
			_container->construct( _handle, std::forward<ARGS>(args)... );
		}

		void destruct() {
			static_assert(C == MUTAB, "called destruct() on CONST accessor");
			_container->destruct( _handle );
		}

		bool exists() const {
			return _container->exists( _handle );
		}
	};


	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

	public:
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}



	private:
		friend BASE;

		void _increment() {
			if constexpr(Dense) ++_handle;
			else do ++_handle; while( (int)_handle != _container->size() && !_container->exists( _handle ) );
		}

		void _decrement() {
			if constexpr(Dense) --_handle;
			else do --_handle; while( !_container->exists( _handle ) );
		}

		auto _get_comparable() const {  return _handle;  }

		template<Const_Flag CC>
		auto _will_compare_with(const Iterator<CC>& o) const {
			DCHECK_EQ(_container, o._container);
		}
	};









	class Memory_Block :
			private Allocator,
			private Add_num_existing<Count>,
			private Add_exists_bitset<Exists_Bitset> {

		using NUM_EXISTING_BASE = Add_num_existing<Count>;
		using EXISTS_BITSET_BASE = Add_exists_bitset<Exists_Bitset>;

	public:
		using Val = Context::Val;
		using Allocator = Context::Allocator;
		static constexpr int Stack_Buffer = Context::Stack_Buffer;
		static constexpr bool Is_Dense = Context::Dense;
		static constexpr bool Is_Sparse = Context::Sparse;
		static constexpr bool Has_Exists_Inplace = Context::Exists_Inplace;
		static constexpr bool Has_Exists_Bitset = Context::Exists_Bitset;
		static constexpr bool Has_Exists = Context::Exists;
		static constexpr bool Has_Count = Context::Count;

		using Handle = Context::Handle;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;


		static_assert(!(Dense && Count), "no need for COUNT if vector is DENSE");

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

		auto& _get(Handle key)       { return _data[key]; }
		auto& _get(Handle key) const { return _data[key]; }


	public:
		// copy-construct
		Memory_Block(const Memory_Block& o) :
				Allocator(o),
				NUM_EXISTING_BASE(o),
				EXISTS_BITSET_BASE(o),
				_size(o._size) {

			static_assert(Dense || Exists || std::is_trivially_copy_constructible_v<Val>,
				"can't copy-construct non-POD container if no EXISTS or DENSE flags");

			if(_size > Stack_Buffer) {
				_data = std::allocator_traits<Allocator>::allocate(_allocator(), _size);
			}
			else {
				_data = _get_stack_buffer();
			}

			for(int i=0; i<_size; ++i) {
				std::allocator_traits<Allocator>::construct(_allocator(), _data+i);
				if( o.exists(i) ) {
					_get(i).construct( o._get(i) );
				}
			}
		}

		// move-construct
		Memory_Block(Memory_Block&& o) :
				Allocator( std::move(o) ),
				NUM_EXISTING_BASE( std::move(o) ),
				EXISTS_BITSET_BASE( std::move(o) ),
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
			static_assert(!Exists_Inplace || !Exists_Bitset, "can't have both");
			static_assert(!Dense || !Exists, "can't have both");
			static_assert(!Dense || !Count, "can't have both");
		}

		template<class... ARGS>
		Memory_Block(int size, ARGS&&... args) : _size(size) {
			static_assert(Dense || sizeof...(ARGS) == 0, "only DENSE memory_blocks can supply construction args");

			if(_size > Stack_Buffer) {
				_data = std::allocator_traits<Allocator>::allocate(_allocator(), _size);
			}
			else {
				_data = _get_stack_buffer();
			}

			for(int i=0; i<_size; ++i) {
				std::allocator_traits<Allocator>::construct(_allocator(), _data+i);
				if constexpr(Dense) {
					_get(i).construct( args... );
				}
			}

			if constexpr(Exists_Bitset) EXISTS_BITSET_BASE::exists.resize( _size );
		}

		~Memory_Block() {
			if constexpr(Dense || Exists) {
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
			static_assert(Dense || Exists || std::is_trivially_destructible_v<Val>,
				"can't destroy non-POD container if no EXISTS or DENSE flags");
				
			_destruct_block(data, size, [this](int i){ return exists(i); });
		}

		// destruct nodes+values
		template<class EXISTS_FUN>
		void _destruct_block(Node* data, int size, EXISTS_FUN&& exists_fun) {
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
			static_assert(Dense || Exists || std::is_trivially_move_constructible_v<Val>,
				"can't resize non-POD container if no EXISTS or DENSE flags");

			if constexpr(std::is_trivially_move_constructible_v<Val>) {
				_resize(new_size, [](int){ return true; });
			}
			else _resize(new_size, [this](int i){ return exists(i); });
		}

		template<class EXISTS_FUN>
		void resize(int new_size, EXISTS_FUN&& exists_fun) {
			static_assert(!Dense && !Exists, "can only supply EXISTS_FUN if not Dense and Exists");

			_resize(new_size, std::forward<EXISTS_FUN>(exists_fun));
		}

	private:
		template<class EXISTS_FUN>
		void _resize(int new_size, EXISTS_FUN&& exists_fun) {

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
							std::allocator_traits<Allocator>::construct(_allocator(), new_data+i, std::move( _data[i] ));
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

				_destruct_block(_data, _size, std::forward<EXISTS_FUN>(exists_fun));
			}
			else {
				// same memory location
				_destruct_block(_data + n, _size - n, std::forward<EXISTS_FUN>(exists_fun));
			}

			// construct new nodes
			for(int i=_size; i<new_size; ++i) {
				std::allocator_traits<Allocator>::construct(_allocator(), new_data+i);
				if constexpr(Dense) {
					new_data[i].construct(); // todo: supply args
				}
			}


			// remove old block
			if(_size > Stack_Buffer) {
				std::allocator_traits<Allocator>::deallocate(_allocator(), _data, _size);
			}

			_data = new_data;
			_size = new_size;

			if constexpr(Exists_Bitset) EXISTS_BITSET_BASE::exists.resize( new_size );
		}


	public:
		auto size() const { return _size; }



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		template<class... ARGS>
		void construct(Handle key, ARGS&&... args) {
			static_assert(!Dense, "construct() not supported for DENSE memory-blocks");

			_check_bounds(key);
			if constexpr(Exists) {
				DCHECK( !exists(key) ) << "element already constructed";
				_set_exists(key, true);
			}
			_get(key).construct( std::forward<ARGS>(args)... );
			if constexpr(Count) ++NUM_EXISTING_BASE::num_existing;
		}

		void destruct(Handle key) {
			static_assert(!Dense, "destruct() not supported for DENSE memory-blocks");

			_check_bounds(key);
			if constexpr(Exists) {
				DCHECK( exists(key) ) << "erasing already erased element";
				_set_exists(key, false);
			}
			_get(key).destruct();
			if constexpr(Count) --NUM_EXISTING_BASE::num_existing;
		}

		bool exists(Handle key) const {
			_check_bounds(key);
			static_assert(Dense || Exists, "called exists() on object without EXISTS or DENSE");

			if constexpr(Dense) {
				return true;
			} else if constexpr(Exists_Inplace) {
				return _get(key).exists;
			} else if constexpr(Exists_Bitset) {
				return EXISTS_BITSET_BASE::exists[key];
			}
		}

		Val& operator[](Handle key) {
			_check_bounds(key);
			if constexpr(Exists) DCHECK( exists(key) ) << "accessing non-existing element";
			return _get(key);
		}

		const Val& operator[](Handle key) const {
			_check_bounds(key);
			if constexpr(Exists) DCHECK( exists(key) ) << "accessing non-existing element";
			return _get(key);
		}


		template<class... ARGS>
		void construct_all(const ARGS&... args) {
			static_assert(!Dense, "construct_all() not supported for DENSE memory-blocks");

			for(int i=0; i<_size; ++i) {
				if constexpr(Exists) {
					DCHECK( !exists(i) ) << "can't construct_all() if some elements already exist";
				}
				construct(i, args...);
			}
		}





		//
		// interface
		//
	public:
		auto operator()(Handle key) {
			_check_bounds(key);
			return Accessor<MUTAB>(this, key);
		}

		auto operator()(Handle key) const {
			_check_bounds(key);
			return Accessor<CONST>(this, key);
		}


		int count() const {
			static_assert(Countable);
			if constexpr(Dense) return size();
			else return NUM_EXISTING_BASE::num_existing;
		}

		/* ambiguous - what does it mean?
		bool empty() const {
			static_assert(Countable);
			return size() == 0;
		}
		*/




	public:
		auto begin() {
			static_assert(Iterable);
			auto e = Iterator<MUTAB>(this, 0);
			if(_size && !e.accessor().exists()) ++e;
			return e;
		}

		auto begin() const {
			static_assert(Iterable);
			auto e = Iterator<CONST>(this, 0);
			if(_size && !e.accessor().exists()) ++e;
			return e;
		}


		auto end() {
			static_assert(Iterable);
			return Iterator<MUTAB>(this, _size);
		}

		auto end() const {
			static_assert(Iterable);
			return Iterator<CONST>(this, _size);
		}




	private:
		void _set_exists(Handle key, bool new_exists) {
			static_assert(Exists);

			if constexpr(Exists_Inplace) {
				_get(key).exists = new_exists;
			}
			else if constexpr(Exists_Bitset) {
				EXISTS_BITSET_BASE::exists[key] = new_exists;
			}
		}

		void _check_bounds(Handle key) const {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, _size ) << "index out of bounds";
		}

	};







	struct With_Builder : Memory_Block {
		FORWARDING_CONSTRUCTOR(With_Builder, Memory_Block) {}

		template<class NEW_ALLOCATOR>
		using ALLOCATOR =
			typename Context< Val, NEW_ALLOCATOR, Stack_Buffer, Dense, Exists_Inplace, Exists_Bitset, Count > :: With_Builder;

		template<int NEW_STACK_BUFFER>
		using INPLACE_BUFFER =
			typename Context< Val, Allocator, NEW_STACK_BUFFER, Dense, Exists_Inplace, Exists_Bitset, Count > :: With_Builder;


		using DENSE =
			typename Context< Val, Allocator, Stack_Buffer, true, Exists_Inplace, Exists_Bitset, Count > :: With_Builder;

		using SPARSE = // (default)
			typename Context< Val, Allocator, Stack_Buffer, false, Exists_Inplace, Exists_Bitset, Count > :: With_Builder;


		using EXISTS_INPLACE =
			typename Context< Val, Allocator, Stack_Buffer, Dense, true, false, Count > :: With_Builder;

		using EXISTS_BITSET =
			typename Context< Val, Allocator, Stack_Buffer, Dense, false, true, Count > :: With_Builder;

		using EXISTS = EXISTS_BITSET; // seems to be faster than inplace version

		using COUNT =
			typename Context< Val, Allocator, Stack_Buffer, Dense, Exists_Inplace, Exists_Bitset, true > :: With_Builder;

		using FULL_BLOWN =
			typename Context< Val, Allocator, Stack_Buffer, Dense, false, true, true > :: With_Builder; // by default bitset-exists
	};




}; // struct Context
} // namespace Memory_Block
} // namespace internal






template<
	class T
>
using Memory_Block = typename internal::Memory_Block::Context<
	T,
	std::allocator<T>, // ALLCOATOR
	0, // STACK_BUFFER
	false, // DENSE
	false, // EXISTS_INPLACE
	false, // EXISTS_BITSET
	false  // COUNT
> :: With_Builder;











}
