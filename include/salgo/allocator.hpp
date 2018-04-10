#pragma once

#include "memory-block.hpp"
#include "vector.hpp"
#include "int-handle.hpp"

namespace salgo {









namespace internal {
namespace Allocator {




//
// handles
//
struct Small_Handle;
template<class,class> struct Handle_T;

struct Small_Handle : Int_Handle<unsigned int, Small_Handle> {
	using BASE = Int_Handle<unsigned int, Small_Handle>;

	FORWARDING_CONSTRUCTOR(Small_Handle, BASE);
	
	template<class A, class B>
	Small_Handle( Handle_T<A,B> h ) : BASE((h.a << 24) | h.b) {}
};

template<class A, class B>
struct Handle_T {
	A a = A(-1);
	B b;

	Handle_T() = default;
	Handle_T(const Handle_T&) = default;

	Handle_T(A aa, B bb) : a(aa), b(bb) {
		DCHECK_GE(a, 0); DCHECK_LT(a, 1<<8);
		DCHECK_GE(b, 0); DCHECK_LT(b, 1<<24);
	}

	Handle_T( Small_Handle h ) : a(h>>24), b(h&0xffffff) {}

	bool valid() const { return a != -1; }

	void reset() { a = A(-1); }

	bool operator==(const Handle_T& o) const { return a==o.a && b==o.b; }
	bool operator!=(const Handle_T& o) const { return !(*this == o); }
};

using Handle = Handle_T<int, int>;





template<
	class _VAL,
	bool _AUTO_DESTRUCT
>
struct Context {

	using Val = _VAL;
	static constexpr bool Auto_Destruct = _AUTO_DESTRUCT;

	using Small_Handle = Allocator::Small_Handle;
	using Handle = Allocator::Handle;

	using Memory_Block = std::conditional_t<
		Auto_Destruct,
		typename salgo::Memory_Block<Val> :: EXISTS,
		salgo::Memory_Block<Val>
	>;









	// forward
	class Allocator;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		Handle handle() const { return _handle; }

		auto& val()       { return _owner[_handle]; }
		auto& val() const { return _owner[_handle]; }

		void destruct() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.destruct( _handle );
		}


	private:
		Accessor(Const<Allocator,C>& owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Allocator;


	private:
		Const<Allocator,C>& _owner;
		const Handle _handle;
	};






	class Allocator {
	private:
		salgo::Vector< Memory_Block > v;
		int current_filled = 0;
		int current_max_size = 0;

	public:
		using Val = Context::Val;
		using Small_Handle = Context::Small_Handle;
		using       Handle = Context::      Handle;

		//Allocator() : v( 1, 1 ) {}


	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) {
			if(current_filled >= current_max_size) {
				current_max_size = (current_max_size+1) << 1;
				v.emplace_back( current_max_size );
				current_filled = 0;
			}

			v.back()().construct( current_filled, std::forward<ARGS>(args)... );

			Handle handle = {v.size()-1, current_filled++};

			return Accessor<MUTAB>( *this, handle );
		}


		void destruct( Small_Handle h )  { destruct( Handle(h) ); }
		void destruct(   Handle h )  { v[h.a].destruct( h.b ); }

		auto& operator[]( Small_Handle h )       { return operator[]( Handle(h) ); }
		auto& operator[]( Small_Handle h ) const { return operator[]( Handle(h) ); }

		auto& operator[]( Handle h )       { return v[h.a][h.b]; }
		auto& operator[]( Handle h ) const { return v[h.a][h.b]; }


		auto operator()( Small_Handle h )       { return operator()( Handle(h) ); };
		auto operator()( Small_Handle h ) const { return operator()( Handle(h) ); };

		auto operator()( Handle h )       { return Accessor<MUTAB>(*this, h); }
		auto operator()( Handle h ) const { return Accessor<CONST>(*this, h); }
	};



	struct With_Builder : Allocator {

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL, Auto_Destruct> :: With_Builder;

		using AUTO_DESTRUCT = typename
			Context<Val, true> :: With_Builder;

		// hack to avoid circular dependency in List
		//template<class>
		//using HANDLE_FOR_VAL = Handle;
	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template<
	class VAL
>
using Allocator = typename internal::Allocator::Context<
	VAL,
	false // auto-destruct
>::With_Builder;








} // namespace salgo




