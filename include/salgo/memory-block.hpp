#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "stack-storage.hpp"
#include "accessors.hpp"
#include "handles.hpp"

#include <cstring> // memcpy

#include "helper-macros-on"

namespace salgo {






	namespace internal {
		namespace memory_block {


			template<bool> struct Add_num_existing { int num_existing = 0; };
			template<> struct Add_num_existing<false> {};


			template<bool> struct Add_exists { bool exists = false; };
			template<> struct Add_exists<false> {};


			template<bool> struct Add_exists_bitset {
				std::vector<bool> exists; // TODO: don't store size and capacity here
			};
			template<> struct Add_exists_bitset<false> {};





			using Handle_Int_Type = int;


			template<class X>
			struct Handle : Int_Handle_Base<Handle<X>, Handle_Int_Type> {
				using BASE = Int_Handle_Base<Handle<X>, Handle_Int_Type>;
				EXPLICIT_FORWARDING_CONSTRUCTOR( Handle, BASE ) {} // explicit, because we don't want automatic creation from `int`

				//Handle() = default;
				//explicit Handle(int val) : BASE(val) {}
			};

			template<class X>
			using Handle_Small = Handle<X>;


			// same as Handle, but allow creation from `int`
			template<class X>
			struct Index : Handle<X> {
				FORWARDING_CONSTRUCTOR(Index, Handle<X>) {}
			};








			template<class _VAL, class _ALLOCATOR, int _STACK_BUFFER, bool _DENSE,
					bool _CONSTRUCTED_FLAGS_INPLACE, bool _CONSTRUCTED_FLAGS_BITSET, bool _COUNT>
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
				static constexpr bool Exists_Inplace = _CONSTRUCTED_FLAGS_INPLACE;
				static constexpr bool Exists_Bitset = _CONSTRUCTED_FLAGS_BITSET;
				static constexpr bool Exists = _CONSTRUCTED_FLAGS_BITSET || _CONSTRUCTED_FLAGS_INPLACE;
				static constexpr bool Count = _COUNT;



				static constexpr bool Iterable = Exists || Dense;
				static constexpr bool Countable = Count || Dense;





				using Handle       = memory_block::Handle<Context>;
				using Handle_Small = memory_block::Handle_Small<Context>;
				using Index        = memory_block::Index<Context>;





				struct Node : salgo::Stack_Storage<Val>, Add_exists<Exists_Inplace> {};






				//
				// accessor
				//
				template<Const_Flag C>
				class Accessor : public Accessor_Base<C,Context> {
					using BASE = Accessor_Base<C,Context>;
					using BASE::container;
					using BASE::_handle;

				public:
					FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

				public:
					auto index() const { return BASE::handle(); }



					template<class... ARGS>
					auto& construct(ARGS&&... args) {
						static_assert(C == MUTAB, "called construct() on CONST accessor");
						static_assert(!Dense, "construct() not supported for DENSE memory-blocks");

						_check_bounds();
						if constexpr(Exists) {
							DCHECK( !constructed() ) << "element already constructed";
							CONT._set_exists(_handle(), true);
						}
						_get().construct( std::forward<ARGS>(args)... );
						if constexpr(Count) ++CONT.num_existing;

						return *this;
					}

					auto& destruct() {
						static_assert(C == MUTAB, "called destruct() on CONST accessor");
						static_assert(!Dense, "destruct() not supported for DENSE memory-blocks");

						_check_bounds();
						if constexpr(Exists) {
							DCHECK( constructed() ) << "erasing already erased element";
							CONT._set_exists(_handle(), false);
						}
						_get().destruct();
						if constexpr(Count) --CONT.num_existing;

						return *this;
					}


					// assumes key is in bounds
					bool constructed() const {
						_check_bounds();
						static_assert(Dense || Exists, "called constructed() on object without CONSTRUCTED_FLAGS or DENSE");

						if constexpr(Dense) {
							return true;
						} else if constexpr(Exists_Inplace) {
							return _get().exists;
						} else if constexpr(Exists_Bitset) {
							return CONT.exists[ _handle() ];
						}
					}

					// same as `constructed()` but also checks bounds
					bool exists_SLOW() const {
						if(!_is_in_bounds()) return false;
						return constructed();
					}

				private:
					bool _is_in_bounds() const {
						return CONT._is_in_bounds( _handle() );
					}

					void _check_bounds() const {
						CONT._check_bounds( _handle() );
					}

					auto& _get()       { return CONT._get( _handle() ); }
					auto& _get() const { return CONT._get( _handle() ); }
				};




				class End_Iterator {};



				//
				// iterator
				//
				template<Const_Flag C>
				class Iterator : public Iterator_Base<C,Context> {
					using BASE = Iterator_Base<C,Context>;
					using BASE::container;
					using BASE::_handle;

				public:
					FORWARDING_CONSTRUCTOR(Iterator, BASE) {}



				private:
					friend BASE;

					void _increment() {
						if constexpr(Dense) ++_handle();
						else do ++HANDLE; while( (int)_handle() != CONT.domain() && !BASE::accessor().constructed() );
					}

					void _decrement() {
						if constexpr(Dense) --_handle();
						else do --HANDLE; while( !BASE::accessor().constructed() );
					}

				public:
					bool operator!=(const End_Iterator&) { return _handle() != CONT.domain(); }
				};











				class Memory_Block :
						private Allocator,
						private Add_num_existing<Count>,
						private Add_exists_bitset<Exists_Bitset> {

					using NUM_EXISTING_BASE = Add_num_existing<Count>;
					using CONSTRUCTED_FLAGS_BITSET_BASE = Add_exists_bitset<Exists_Bitset>;

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

					using Handle       = Context::Handle;
					using Handle_Small = Context::Handle_Small;
					using Index        = Context::Index;


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

					auto& _get(Index key)       { return _data[key]; }
					auto& _get(Index key) const { return _data[key]; }


				public:
					// copy-construct
					Memory_Block(const Memory_Block& o) :
							Allocator(o),
							NUM_EXISTING_BASE(o),
							CONSTRUCTED_FLAGS_BITSET_BASE(o),
							_size(o._size) {

						static_assert(Dense || Exists || std::is_trivially_copy_constructible_v<Val>,
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

						if constexpr(Exists_Bitset) CONSTRUCTED_FLAGS_BITSET_BASE::exists.resize( _size );
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
						static_assert(Dense || Exists || std::is_trivially_move_constructible_v<Val>,
									  "can't resize non-POD container if no CONSTRUCTED_FLAGS or DENSE flags");

						if constexpr(std::is_trivially_move_constructible_v<Val>) {
							_resize(new_size, [](int){ return true; });
						}
						else _resize(new_size, [this](int i){ return (*this)(i).constructed(); });
					}

					template<class CONSTRUCTED_FLAGS_FUN>
					void resize(int new_size, CONSTRUCTED_FLAGS_FUN&& exists_fun) {
						static_assert(!Dense && !Exists, "can only supply CONSTRUCTED_FLAGS_FUN if not Dense and Exists");

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

							_destruct_block(_data, _size, std::forward<CONSTRUCTED_FLAGS_FUN>(exists_fun));
						}
						else {
							// same memory location
							_destruct_block(_data + n, _size - n, std::forward<CONSTRUCTED_FLAGS_FUN>(exists_fun));
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

						if constexpr(Exists_Bitset) CONSTRUCTED_FLAGS_BITSET_BASE::exists.resize( new_size );
					}


				public:
					auto domain() const { return _size; }

					auto size() const {
						static_assert(Dense, "size() is ambiguous when memory block is Sparse");
						return _size;
					}




					// direct access
				public:
					Val& operator[](Index key) {
						_check_bounds(key);
						if constexpr(Exists) DCHECK( (*this)(key).constructed() ) << "accessing non-existing element with key " << key;
						return _get(key).get();
					}

					const Val& operator[](Index key) const {
						_check_bounds(key);
						if constexpr(Exists) DCHECK( (*this)(key).constructed() ) << "accessing non-existing element with key " << key;
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
						return Accessor<MUTAB>(this, key);
					}

					auto operator()(Index key) const {
						_check_bounds(key);
						return Accessor<CONST>(this, key);
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
						static_assert(!Dense, "construct_all() not supported for DENSE memory-blocks");

						for(int i=0; i<_size; ++i) {
							if constexpr(Exists) {
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
						static_assert(Countable);
						if constexpr(Dense) return domain();
						else return NUM_EXISTING_BASE::num_existing;
					}

					bool empty() const {
						static_assert(Countable);
						return count() == 0;
					}




				public:
					auto begin() {
						static_assert(Iterable);
						auto e = Iterator<MUTAB>(this, Index(0));
						if(_size && !e.accessor().constructed()) ++e;
						return e;
					}

					auto begin() const {
						static_assert(Iterable);
						auto e = Iterator<CONST>(this, Index(0));
						if(_size && !e.accessor().constructed()) ++e;
						return e;
					}

					auto end() const {
						static_assert(Iterable);
						return End_Iterator();
					}




				private:
					void _set_exists(Handle key, bool new_exists) {
						static_assert(Exists);

						if constexpr(Exists_Inplace) {
							_get(key).exists = new_exists;
						}
						else if constexpr(Exists_Bitset) {
							CONSTRUCTED_FLAGS_BITSET_BASE::exists[key] = new_exists;
						}
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


					using CONSTRUCTED_FLAGS_INPLACE =
					typename Context< Val, Allocator, Stack_Buffer, Dense, true, false, Count > :: With_Builder;

					using CONSTRUCTED_FLAGS_BITSET =
					typename Context< Val, Allocator, Stack_Buffer, Dense, false, true, Count > :: With_Builder;

					using CONSTRUCTED_FLAGS = CONSTRUCTED_FLAGS_BITSET; // seems to be faster than inplace version

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
	using Memory_Block = typename internal::memory_block::Context<
			T,
			std::allocator<T>, // ALLCOATOR
			0, // STACK_BUFFER
			false, // DENSE
			false, // CONSTRUCTED_FLAGS_INPLACE
			false, // CONSTRUCTED_FLAGS_BITSET
			false  // COUNT
	> :: With_Builder;











} // namespace salgo


#include "helper-macros-off"




template<class X>
struct std::hash<salgo::internal::memory_block::Handle<X>> {
	size_t operator()(const salgo::internal::memory_block::Handle<X>& h) const {
		return std::hash<salgo::internal::memory_block::Handle_Int_Type>()(
			salgo::internal::memory_block::Handle_Int_Type(h)
		);
	}
};

