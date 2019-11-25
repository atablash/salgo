#pragma once

#include "common.hpp"

#include <glog/logging.h>





namespace salgo {
namespace internal {
namespace inplace_storage {

template<
	class _T,
	bool _TREAT_AS_POD,
	bool _TREAT_AS_VOID,
	bool _PERSISTENT
>
struct Context {

	using T = _T;
	static constexpr bool Treat_As_Pod = _TREAT_AS_POD;
	static constexpr bool Treat_As_Void = _TREAT_AS_VOID;
	static constexpr bool Persistent = _PERSISTENT;



	//
	// non-trivial T
	//
	// it doesn't know if the object is constructed or not
	// you have to destruct the object manually
	// in debug mode this is checked using `constructed` field
	//
	class Inplace_Storage__nt {
	private:
		char data[ sizeof(T) ];
		#ifndef NDEBUG
		bool constructed = false;
		#endif

		static_assert(Treat_As_Pod + Treat_As_Void + Persistent <= 1, "can have max 1");

	public:
		Inplace_Storage__nt() = default;
		
		//
		// don't know if `o` is constructed or not...
		//
		// for non-trivial types, construction and assignments are only valid if objects are constructed
		// (we don't have a way to know this, but checks are performed in debug mode)
		//
		Inplace_Storage__nt(const Inplace_Storage__nt& o) {
			#ifndef NDEBUG
			DCHECK(o.constructed);
			DCHECK(!constructed);
			#endif

			construct(*(const T*)&o.data[0]);
		}

		Inplace_Storage__nt(Inplace_Storage__nt&& o) {
			#ifndef NDEBUG
			DCHECK(o.constructed);
			DCHECK(!constructed);
			#endif

			construct( std::move( *(T*)&o.data[0] ) );
		}

		Inplace_Storage__nt& operator=(const Inplace_Storage__nt& o) {
			#ifndef NDEBUG
			DCHECK(o.constructed);
			DCHECK(constructed);
			#endif

			*(T*)&data[0] = *(const T*)&o.data[0];

			return *this;
		}




		// this causes ambiguity for no args - don't know if should construct or not:
		//template<class... ARGS>
		//Inplace_Storage__nt(ARGS&&... args) {
		//	construct(std::forward<ARGS>(args)...);
		//}

		#ifndef NDEBUG
		~Inplace_Storage__nt() {
			if constexpr( !std::is_trivially_destructible_v<T> ) DCHECK(!constructed);
		}
		#endif

		template<class... ARGS>
		void construct(ARGS&&... args) {

			#ifndef NDEBUG
			DCHECK(!constructed);
			constructed = true;
			#endif

			new (&get()) T( std::forward<ARGS>(args)... );
		}

		void destruct() {

			get().~T();

			#ifndef NDEBUG
			DCHECK(constructed);
			constructed = false;
			#endif
		}



		inline operator T&() {
			return get();
		}

		inline operator const T&() const {
			return get();
		}



		inline T& get() {

			#ifndef NDEBUG
			DCHECK(constructed);
			#endif

			return *reinterpret_cast<T*>( &data[0] );
		}

		inline const T& get() const {

			#ifndef NDEBUG
			DCHECK(constructed);
			#endif

			return *reinterpret_cast<const T*>( &data[0] );
		}
	};








	// trivial T
	class Inplace_Storage__t {
	private:
		char data[ sizeof(T) ];
		#ifndef NDEBUG
		bool constructed = false;
		#endif

		static_assert(Treat_As_Pod + Treat_As_Void + Persistent <= 1, "can have max 1");

	public:
		template<class... ARGS>
		void construct(ARGS&&... args) {
			new (&get()) T( std::forward<ARGS>(args)... );
		}

		void destruct() { static_assert(std::is_trivially_destructible_v<T>); }


		operator auto&()       { return get(); }
		operator auto&() const { return get(); }


		T& get() {
			return *reinterpret_cast<T*>( &data[0] );
		}

		const T& get() const {
			return *reinterpret_cast<const T*>( &data[0] );
		}
	};




	// noop copying
	class Inplace_Storage__noop {
	private:
		char data[ sizeof(T) ];

		static_assert(Treat_As_Pod + Treat_As_Void + Persistent <= 1, "can have max 1");

	public:
		Inplace_Storage__noop() {}
		Inplace_Storage__noop(const Inplace_Storage__noop&) {}
		Inplace_Storage__noop(Inplace_Storage__noop&&) {}
		Inplace_Storage__noop& operator=(const Inplace_Storage__noop&) { return *this; }
		Inplace_Storage__noop& operator=(Inplace_Storage__noop&&) { return *this; }

		bool operator==(const Inplace_Storage__noop& o) const { return get() == o.get(); }
		bool operator!=(const Inplace_Storage__noop& o) const { return get() != o.get(); }


	public:
		template<class... ARGS>
		void construct(ARGS&&... args) {
			new (&get()) T( std::forward<ARGS>(args)... );
		}

		void destruct() {}


		operator auto&()       { return get(); }
		operator auto&() const { return get(); }


		T& get() {
			return *reinterpret_cast<T*>( &data[0] );
		}

		const T& get() const {
			return *reinterpret_cast<const T*>( &data[0] );
		}
	};









	using Inplace_Storage = std::conditional_t<
		Treat_As_Void,
		Inplace_Storage__noop,
		std::conditional_t<
			Treat_As_Pod || std::is_trivially_move_constructible_v<T>,
			Inplace_Storage__t,
			Inplace_Storage__nt
		>
	>;




	// forward declaration
	struct With_Builder_Persistent;



	struct With_Builder : Inplace_Storage {
		FORWARDING_CONSTRUCTOR(With_Builder, Inplace_Storage) {}

		using TREAT_AS_POD =
			typename Context<T, true, Treat_As_Void, Persistent> ::With_Builder;

		using TREAT_AS_VOID =
			typename Context<T, Treat_As_Pod, true, Persistent> ::With_Builder;

		using PERSISTENT =
			typename Context<T, Treat_As_Pod, Treat_As_Void, true> ::With_Builder_Persistent;
	};


	struct With_Builder_Persistent : With_Builder {
		With_Builder_Persistent() { static_assert(Persistent); }
		With_Builder_Persistent(const With_Builder_Persistent&) = delete;
		With_Builder_Persistent(With_Builder_Persistent&&) = delete;
		With_Builder_Persistent& operator=(const With_Builder_Persistent&) = delete;
		With_Builder_Persistent& operator=(With_Builder_Persistent&&) = delete;
	};



}; // struct Context


} // namespace Inplace_Storage
} // namespace internal



template<class T>
using Inplace_Storage = typename internal::inplace_storage::Context<
	T,
	false, // treat as pod
	false, // tread as void
	false  // persistent
> :: With_Builder;




} // namespace salgo


