#pragma once

#include <glog/logging.h>





namespace salgo {






//
// it doesn't know if the object is constructed or not
// you have to destruct the object manually
// in debug mode this is checked using `constructed` field
//
template<class T>
class Stack_Storage {
private:
	char data[ sizeof(T) ];
	#ifndef NDEBUG
	bool constructed = false;
	#endif


public:
	Stack_Storage() {}

	template<class... ARGS>
	Stack_Storage(ARGS&&... args) {
		construct(std::forward<ARGS>(args)...);
	}

	~Stack_Storage() {
		#ifndef NDEBUG
		DCHECK(!constructed);
		#endif
	}

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





} // namespace salgo


