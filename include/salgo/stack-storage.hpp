#pragma once

#include "common.hpp"

#include <glog/logging.h>





namespace salgo {
namespace internal {
namespace Stack_Storage {

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

	static_assert(!Treat_As_Pod || !Treat_As_Void, "can't have both");



	//
	// non-trivial T
	//
	// it doesn't know if the object is constructed or not
	// you have to destruct the object manually
	// in debug mode this is checked using `constructed` field
	//
	class Stack_Storage__nt {
	private:
		char data[ sizeof(T) ];
		#ifndef NDEBUG
		bool constructed = false;
		#endif


	public:
		Stack_Storage__nt() = default;
		
		//
		// don't know if `o` is constructed or not...
		//
		// for non-trivial types, construction and assignments are only valid if objects are constructed
		// (we don't have a way to know this, but checks are performed in debug mode)
		//
		Stack_Storage__nt(const Stack_Storage__nt& o) {
			#ifndef NDEBUG
			DCHECK(o.constructed);
			DCHECK(!constructed);
			#endif

			construct(*(const T*)&o.data[0]);
		}

		Stack_Storage__nt(Stack_Storage__nt&& o) {
			#ifndef NDEBUG
			DCHECK(o.constructed);
			DCHECK(!constructed);
			#endif

			construct( std::move( *(T*)&o.data[0] ) );
		}

		Stack_Storage__nt& operator=(const Stack_Storage__nt& o) {
			#ifndef NDEBUG
			DCHECK(o.constructed);
			DCHECK(constructed);
			#endif

			*(T*)&data[0] = *(const T*)&o.data[0];

			return *this;
		}




		// this causes ambiguity for no args - don't know if should construct or not:
		//template<class... ARGS>
		//Stack_Storage__nt(ARGS&&... args) {
		//	construct(std::forward<ARGS>(args)...);
		//}

		#ifndef NDEBUG
		~Stack_Storage__nt() {
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
	class Stack_Storage__t {
	private:
		char data[ sizeof(T) ];
		#ifndef NDEBUG
		bool constructed = false;
		#endif


	public:
		template<class... ARGS>
		void construct(ARGS&&... args) {
			new (&get()) T( std::forward<ARGS>(args)... );
		}

		void destruct() {}


		inline operator auto&()       { return get(); }
		inline operator auto&() const { return get(); }


		inline T& get() {
			return *reinterpret_cast<T*>( &data[0] );
		}

		inline const T& get() const {
			return *reinterpret_cast<const T*>( &data[0] );
		}
	};




	// noop copying
	class Stack_Storage__noop {
	private:
		char data[ sizeof(T) ];

	public:
		Stack_Storage__noop() {}
		Stack_Storage__noop(const Stack_Storage__noop&) {}
		Stack_Storage__noop(Stack_Storage__noop&&) {}
		Stack_Storage__noop& operator=(const Stack_Storage__noop&) { return *this; }
		Stack_Storage__noop& operator=(Stack_Storage__noop&&) { return *this; }


	public:
		template<class... ARGS>
		void construct(ARGS&&... args) {
			new (&get()) T( std::forward<ARGS>(args)... );
		}

		void destruct() {}


		inline operator auto&()       { return get(); }
		inline operator auto&() const { return get(); }


		inline T& get() {
			return *reinterpret_cast<T*>( &data[0] );
		}

		inline const T& get() const {
			return *reinterpret_cast<const T*>( &data[0] );
		}
	};








	// no copies
	class Stack_Storage__persistent {
	private:
		char data[ sizeof(T) ];

	public:
		Stack_Storage__persistent() {}
		//#ifdef NDEBUG
		Stack_Storage__persistent(const Stack_Storage__persistent&) = delete;
		Stack_Storage__persistent(Stack_Storage__persistent&&) = delete;
		Stack_Storage__persistent& operator=(const Stack_Storage__persistent&) = delete;
		Stack_Storage__persistent& operator=(Stack_Storage__persistent&&) = delete;
		//#else
		/*
		Stack_Storage__persistent(const Stack_Storage__persistent&) { static_assert(false, "persistent"); }
		Stack_Storage__persistent(Stack_Storage__persistent&&) { static_assert(false, "persistent"); }
		Stack_Storage__persistent& operator=(const Stack_Storage__persistent&) { static_assert(false, "persistent"); }
		Stack_Storage__persistent& operator=(Stack_Storage__persistent&&) { static_assert(false, "persistent"); }
		*/
		//#endif

	public:
		template<class... ARGS>
		void construct(ARGS&&... args) {
			new (&get()) T( std::forward<ARGS>(args)... );
		}

		void destruct() {}


		inline operator auto&()       { return get(); }
		inline operator auto&() const { return get(); }


		inline T& get() {
			return *reinterpret_cast<T*>( &data[0] );
		}

		inline const T& get() const {
			return *reinterpret_cast<const T*>( &data[0] );
		}
	};










	using Stack_Storage = std::conditional_t<
		Treat_As_Void,
		Stack_Storage__noop,
		std::conditional_t<
			Persistent,
			Stack_Storage__persistent,
			std::conditional_t<
				std::is_trivially_copy_constructible_v<T> || Treat_As_Pod,
				Stack_Storage__t,
				Stack_Storage__nt
			>
		>
	>;





	struct With_Builder : Stack_Storage {
		FORWARDING_CONSTRUCTOR(With_Builder, Stack_Storage);

		using TREAT_AS_POD =
			typename Context<T, true, false, false>::With_Builder;

		using TREAT_AS_VOID =
			typename Context<T, false, true, false>::With_Builder;

		using PERSISTENT =
			typename Context<T, false, false, true>::With_Builder;
	};



}; // struct Context


} // namespace Stack_Storage
} // namespace internal



template<class T>
using Stack_Storage = typename internal::Stack_Storage::Context<
	T,
	false, // treat as pod
	false, // tread as void
	false  // persistent
> :: With_Builder;




} // namespace salgo


