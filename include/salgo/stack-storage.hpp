#pragma once

#include <glog/logging.h>





namespace salgo {






//
// it doesn't know if the object is constructed or not
// you have to destruct the object manually
// in debug mode this is checked using `constructed` field
//
template<class T>
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







//
// it doesn't know if the object is constructed or not
// you have to destruct the object manually
// in debug mode this is checked using `constructed` field
//
template<class T>
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




template<class T>
using Stack_Storage = std::conditional_t<
	std::is_trivially_copy_constructible_v<T>,
	Stack_Storage__t <T>,
	Stack_Storage__nt<T>
>;




} // namespace salgo


