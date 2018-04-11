#pragma once


namespace salgo {


//
// provides unique global instance of type T
//
// you can supply optional constructor arguments (but these will be used only the first time!)
//
template<class T, class... ARGS>
inline auto& global_instance(ARGS&&... args) {
	static T t( std::forward<ARGS>(args)... );
	return t;
}




} // namespace salgo

