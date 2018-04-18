#pragma once

#include "pointer-handle.hpp"

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
namespace Salgo_From_Std_Allocator {





template<
	class _ALLOCATOR
>
struct Context {

	using Allocator = _ALLOCATOR;
	using Pointer = typename std::allocator_traits<Allocator>::pointer;


	struct Handle : Pointer_Handle< Pointer, Handle> {
		using BASE = Pointer_Handle< Pointer, Handle>;
		FORWARDING_CONSTRUCTOR(Handle, BASE) {}
	};

	//
	// forward declarations
	//
	template<Const_Flag C> class Accessor;
	class Salgo_From_Std_Allocator;






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		// get HANDLE
		auto   handle() const { return _handle; }
		operator auto() const { return handle(); }

		// get VAL
		auto& operator()()       { return _owner[_handle]; }
		auto& operator()() const { return _owner[_handle]; }
		operator auto&()       { return operator()(); }
		operator auto&() const { return operator()(); }


		void destruct() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.destruct( _handle );
		}


	private:
		Accessor(Const<Salgo_From_Std_Allocator,C>& owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Salgo_From_Std_Allocator;


	private:
		Const<Salgo_From_Std_Allocator,C>& _owner;
		const Handle _handle;
	};








	class Salgo_From_Std_Allocator : private Allocator {

	#ifndef NDEBUG
		int _num_allocations = 0;
		public: ~Salgo_From_Std_Allocator() {
			DCHECK_EQ(0, _num_allocations);
		}
	#endif

	public:
		using Handle = Context::Handle;
		using Small_Handle = Handle;

		template<class... ARGS>
		auto construct(ARGS&&... args) {

			#ifndef NDEBUG
			++_num_allocations;
			#endif

			Pointer pointer = std::allocator_traits<Allocator>::allocate( _allocator(), 1 );
			std::allocator_traits<Allocator>::construct( _allocator(), pointer, std::forward<ARGS>(args)... );
			return Accessor<MUTAB>(*this, pointer);
		}


		template<class... ARGS>
		auto construct_near(Handle h, ARGS&&... args) {

			#ifndef NDEBUG
			++_num_allocations;
			#endif

			Pointer pointer = std::allocator_traits<Allocator>::allocate( _allocator(), 1, h );
			std::allocator_traits<Allocator>::construct( _allocator(), pointer, std::forward<ARGS>(args)... );
			return Accessor<MUTAB>(*this, pointer);
		}


		void destruct(Handle handle) {

			#ifndef NDEBUG
			--_num_allocations;
			#endif
			
			std::allocator_traits<Allocator>::destroy( _allocator(), (Pointer)handle );
			std::allocator_traits<Allocator>::deallocate( _allocator(), handle, 1 );
		}

		auto& operator[]( Handle handle )       { return *(Pointer)handle; }
		auto& operator[]( Handle handle ) const { return *(Pointer)handle; }

		auto operator()( Handle handle )       { return Accessor<MUTAB>(*this, handle); }
		auto operator()( Handle handle ) const { return Accessor<CONST>(*this, handle); }

	private:
		auto& _allocator()       { return *static_cast<      Allocator*>(this); }
		auto& _allocator() const { return *static_cast<const Allocator*>(this); }
	};


	struct With_Builder : Salgo_From_Std_Allocator {

		template<class NEW_VAL>
		using VAL = typename Context<
			typename std::allocator_traits<Allocator>::template rebind_alloc<NEW_VAL>
		> :: With_Builder;

	};



}; // struct Context
}  // namespace Salgo_From_Std_Allocator
}  // namespace internal







template< class ALLOCATOR >
using Salgo_From_Std_Allocator = typename internal::Salgo_From_Std_Allocator::Context<
	ALLOCATOR
>::With_Builder;








} // namespace salgo




