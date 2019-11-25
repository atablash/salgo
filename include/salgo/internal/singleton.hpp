#pragma once


namespace salgo {


//
// derive from Singleton<CRTP> to singletonize a class
//
template<class CRTP>
class Singleton {

	//
	// get instance and optionally pass arguments for creation
	//
	template<class... ARGS>
	static CRTP& operator()(ARGS&&... args) {
		static CRTP t( std::forward<ARGS>(args)... );
		return t;
	}

private:
	// construct only via operator() above
	Singleton() = default;

private:
	// no copies
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
};



} // namespace ab

