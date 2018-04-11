#pragma once

#include "memory-block.hpp"
#include "vector.hpp"
#include "int-handle.hpp"

namespace salgo {









namespace internal {
namespace Allocator {




//
// HANDLES
//
// defined outside Context to specialize std::hash<>
//
// parametrized by unused context X, to make Handles from different Contexts incompatible
//
template<class> struct Handle;
template<class> struct Small_Handle;


// big
using Handle_A = int;
using Handle_B = int;

template<class>
struct Handle {
	Handle_A a = -1;
	Handle_B b;

	Handle() = default;
	Handle(const Handle&) = default;

	Handle(Handle_A aa, Handle_B bb) : a(aa), b(bb) {
		DCHECK_GE(a, 0); DCHECK_LT(a, 1<<8);
		DCHECK_GE(b, 0); DCHECK_LT(b, 1<<24);
	}

	//operator Small_Handle() { return Small_Handle(*this); }

	//Handle( Small_Handle h ) : a(h>>24), b(h&0xffffff) {}

	bool valid() const { return a != -1; }

	void reset() { a = -1; }

	bool operator==(const Handle& o) const { return a==o.a && b==o.b; }
	bool operator!=(const Handle& o) const { return !(*this == o); }
};



// small
template<class X>
struct Small_Handle : Int_Handle<unsigned int, Small_Handle<X>> {
	using BASE = Int_Handle<unsigned int, Small_Handle<X>>;

	Small_Handle() = default;
	Small_Handle(unsigned int v) : BASE(v) {}

	Small_Handle( const Handle<X>& h ) { *this = h; }
	Small_Handle& operator=(const Handle<X>& h) { return *this = (h.a << 24) | h.b; }

	operator Handle<X>() const { return Handle<X>((*this)>>24, (*this)&0xffffff); }
};







template<
	class _VAL,
	bool _AUTO_DESTRUCT,
	bool _SINGLETON
>
struct Context {

	//
	// TEMPLATE PARAMETERS
	//
	using Val = _VAL;
	static constexpr bool Auto_Destruct = _AUTO_DESTRUCT;
	using Singleton = _SINGLETON;

	using Memory_Block = std::conditional_t<
		Auto_Destruct,
		typename salgo::Memory_Block<Val> :: EXISTS,
		salgo::Memory_Block<Val>
	>;





	using       Handle = Allocator::      Handle<Context>;
	using Small_Handle = Allocator::Small_Handle<Context>;
















	// forward
	class Allocator;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		// get handle
		auto     handle() const { return _handle; }
		operator Handle() const { return _handle; }

		auto& operator()()       { return _owner[_handle]; }
		auto& operator()() const { return _owner[_handle]; }

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


		auto& operator[]( Handle h )       { return v[h.a][h.b]; }
		auto& operator[]( Handle h ) const { return v[h.a][h.b]; }

		auto operator()( Handle h )       { return Accessor<MUTAB>(*this, h); }
		auto operator()( Handle h ) const { return Accessor<CONST>(*this, h); }
	};







	class Allocator_Proxy {
		static auto& _get() { return global_instance<Allocator>(); }

	public:
		using          Val = Allocator::Val;
		using Small_Handle = Allocator::Small_Handle;
		using       Handle = Allocator::Handle;

	public:
		template<class... ARGS>	auto construct(ARGS&&... args) { _get(). construct( std::forward<ARGS>(args)... ); }
		template<class... ARGS>	auto  destruct(ARGS&&... args) { _get().  destruct( std::forward<ARGS>(args)... ); }

		template<class... ARGS>	auto& operator[](ARGS&&... args)       { _get().operator[]( std::forward<ARGS>(args)... ); }
		template<class... ARGS>	auto& operator[](ARGS&&... args) const { _get().operator[]( std::forward<ARGS>(args)... ); }

		template<class... ARGS>	auto operator()(ARGS&&... args)       { _get().operator()( std::forward<ARGS>(args)... ); }
		template<class... ARGS>	auto operator()(ARGS&&... args) const { _get().operator()( std::forward<ARGS>(args)... ); }
	};






	using My_Allocator = std::conditional_t<
		Singleton,
		Allocator_Proxy,
		Allocator
	>;




	struct With_Builder : My_Allocator {

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL, Auto_Destruct, Singleton> :: With_Builder;

		using AUTO_DESTRUCT = typename
			Context<Val, true, Singleton> :: With_Builder;

		using SINGLETON =
			Context<Val, Auto_Destruct, true> :: With_Builder;
	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template<
	class VAL
>
using Allocator = typename internal::Allocator::Context<
	VAL,
	false, // auto-destruct
	false // singleton
>::With_Builder;








} // namespace salgo








template<class X>
struct std::hash<salgo::internal::Allocator::Small_Handle<X>> {
	size_t operator()(const salgo::internal::Allocator::Small_Handle<X>& h) const {
		return h;
	}
};

