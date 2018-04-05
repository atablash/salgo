#pragma once


namespace salgo {





/*

STORAGE stores elements of type VAL.
All elements have to be explicitely destructed.

STORAGE concept:
	TEMPLATE ARGUMENTS:
		VAL

	TYPES:
		Handle

	FUNCTIONS:
		construct( args... ) -> Handle
		destruct ( Handle )
		operator[] ( Handle ) -> VAL&

*/







namespace internal {
namespace Allocator_Storage {

template<
	class _VAL,
	class _ALLOCATOR = std::allocator< VAL >
>
struct Context {

	using Val = _VAL;
	using Allocator = _ALLOCATOR;

	using Pointer = std::allocator_traits<Allocator>::pointer;

	struct Handle : Pointer_Handle< Pointer, Handle> {};


	class Allocator_Storage {
	public:
		using Val = Context::Val;
		using Allocator = Context::Allocator;
		
		using Handle = Context::Handle;

		template<class... ARGS>
		Handle construct(ARGS&&... args) {
			Pointer pointer = std::allocator_traits<Allocator>::allocate( Allocator(), 1 );
			std::allocator_traits<Allocator>::construct( Allocator(), pointer, std::forward<ARGS>(args)... );
			return pointer;
		}

		void destruct(Handle handle) {
			std::allocator_traits<Allocator>::destroy( Allocator(), handle );
			std::allocator_traits<Allocator>::deallocate( Allocator(), handle, 1 );
		}

		auto& operator[]( Handle handle )       { return *(Pointer)handle; }
		auto& operator[]( Handle handle ) const { return *(Pointer)handle; }
	};



}; // struct Context
}  // namespace Storage
}  // namespace internal







template< class VAL, class ALLOCATOR >
using Allocator_Storage = internal::Allocator_Storage<VAL,ALLOCATOR>::Allocator_Storage;








} // namespace salgo




