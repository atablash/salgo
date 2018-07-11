#pragma once

#include "memory-block.hpp"
#include "vector.hpp"
#include "handles.hpp"
#include "global-instance.hpp"

namespace salgo {









namespace internal {
namespace crude_allocator {


template<
	class VAL,
	bool AUTO_DESTRUCT,
	bool SINGLETON
>
struct Params {};



//
// HANDLES
//
// defined outside Context to specialize std::hash<>
//
// parametrized by unused context X, to make Handles from different Contexts incompatible
//
template<class> struct Handle;
template<class> struct Handle_Small;

static const int div = 27;

// big
template<class X>
struct Handle : Pair_Handle_Base<Handle<X>, Int_Handle<int,(1<<(32-div))-1>, int> {
	using BASE = Pair_Handle_Base<Handle<X>, Int_Handle<int,(1<<(32-div))-1>, int>;

	Handle() = default;

	template<class A, class B>
	Handle(A aa, B bb) : BASE(aa,bb) {
		DCHECK_GE(aa, 0); DCHECK_LT(aa, 1<<(32-div));
		DCHECK_GE(bb, 0); DCHECK_LT(bb, 1<<div);
	}
};



// small
template<class X>
struct Handle_Small : Int_Handle_Base<Handle_Small<X>, unsigned int> {
	using BASE = Int_Handle_Base<Handle_Small<X>, unsigned int>;


	Handle_Small() = default;
	Handle_Small(unsigned int v) : BASE(v) {}

	Handle_Small( const Handle<X>& h ) { *this = h; }
	Handle_Small& operator=(const Handle<X>& h) {
		DCHECK_LT(h.a, 1<<(32-div));
		DCHECK_LT(h.b, 1<<div);
		*this = (h.a << div) | h.b;
		//LOG(INFO) << h << " -> " << *this;
		return *this;
	}

	operator Handle<X>() const {
		//LOG(INFO) << *this << " -> " << Handle<X>((*this)>>div, (*this)&((1<<div)-1));
		return Handle<X>((*this)>>div, (*this)&((1<<div)-1));
	}
};









template<
	class _VAL,
	bool  _AUTO_DESTRUCT,
	bool  _SINGLETON
>
struct Context {

	using P = Params<_VAL, _AUTO_DESTRUCT, _SINGLETON>;

	//
	// TEMPLATE PARAMETERS
	//
	using Val = _VAL;
	static constexpr bool Auto_Destruct = _AUTO_DESTRUCT;
	static constexpr bool Singleton = _SINGLETON;


	using Memory_Block = std::conditional_t<
		Auto_Destruct,
		typename salgo::Memory_Block<Val> :: CONSTRUCTED_FLAGS,
		salgo::Memory_Block<Val>
	>;





	using       Handle = crude_allocator::      Handle<P>;
	using Handle_Small = crude_allocator::Handle_Small<P>;
















	// forward
	class Crude_Allocator;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		// get handle
		auto     handle() const { return _handle; }
		operator   auto() const { return handle(); }

		auto& operator()()       { return _owner[_handle]; }
		auto& operator()() const { return _owner[_handle]; }
		operator auto&()       { return operator()(); }
		operator auto&() const { return operator()(); }

		void destruct() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.destruct( _handle );
		}


	private:
		Accessor(Const<Crude_Allocator,C>& owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Crude_Allocator;


	private:
		Const<Crude_Allocator,C>& _owner;
		const Handle _handle;
	};






	class Crude_Allocator {
	private:
		salgo::Vector< Memory_Block > v;
		int current_filled = 0;
		int current_max_size = 0;

	public:
		using Val = Context::Val;
		using Handle_Small = Context::Handle_Small;
		using       Handle = Context::      Handle;


	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) {
			if(current_filled >= current_max_size) {
				current_max_size = (current_max_size+1) << 1;
				v.emplace_back( current_max_size );
				current_filled = 0;
			}

			v[LAST](current_filled).construct( std::forward<ARGS>(args)... );

			Handle handle = {v.size()-1, current_filled++};

			return Accessor<MUTAB>( *this, handle );
		}

		template<class... ARGS>
		auto construct_near(Handle, ARGS&&... args) {
			// TODO
			return construct(std::forward<ARGS>(args)...);
		}


		void destruct(       Handle h )  { v[h.a](h.b).destruct(); }

		auto& operator[]( Handle h )       { return v[h.a][h.b]; }
		auto& operator[]( Handle h ) const { return v[h.a][h.b]; }

		auto operator()( Handle h )       { return Accessor<MUTAB>(*this, h); }
		auto operator()( Handle h ) const { return Accessor<CONST>(*this, h); }
	};







	class Allocator_Proxy {
		static auto& _get() { return global_instance<Crude_Allocator>(); }

	public:
		using          Val = typename Crude_Allocator::Val;
		using Handle_Small = typename Crude_Allocator::Handle_Small;
		using       Handle = typename Crude_Allocator::Handle;

	public:
		template<class... ARGS>
		auto construct(ARGS&&... args) { return _get(). construct( std::forward<ARGS>(args)... ); }

		template<class... ARGS>
		auto  destruct(ARGS&&... args) { return _get().  destruct( std::forward<ARGS>(args)... ); }


		template<class... ARGS>
		auto& operator[](ARGS&&... args)       { return _get().operator[]( std::forward<ARGS>(args)... ); }
		
		template<class... ARGS>
		auto& operator[](ARGS&&... args) const { return _get().operator[]( std::forward<ARGS>(args)... ); }


		template<class... ARGS>
		auto operator()(ARGS&&... args)       { return _get().operator()( std::forward<ARGS>(args)... ); }
		
		template<class... ARGS>
		auto operator()(ARGS&&... args) const { return _get().operator()( std::forward<ARGS>(args)... ); }
	};






	using My_Allocator = std::conditional_t<
		Singleton,
		Allocator_Proxy,
		Crude_Allocator
	>;




	struct With_Builder : My_Allocator {

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL, Auto_Destruct, Singleton> :: With_Builder;

		using AUTO_DESTRUCT = typename
			Context<Val, true, Singleton> :: With_Builder;

		using SINGLETON = typename
			Context<Val, Auto_Destruct, true> :: With_Builder;
	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template<
	class VAL
>
using Crude_Allocator = typename internal::crude_allocator::Context<
	VAL,
	false, // auto-destruct
	false // singleton
>::With_Builder;








} // namespace salgo








template<class X>
struct std::hash<salgo::internal::crude_allocator::Handle_Small<X>> {
	size_t operator()(const salgo::internal::crude_allocator::Handle_Small<X>& h) const {
		return h;
	}
};

