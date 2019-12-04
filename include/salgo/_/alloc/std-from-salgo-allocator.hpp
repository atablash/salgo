#pragma once


namespace salgo {






template <class A>
struct Std_From_Salgo_Allocator : private A {

	//using BASE = typename ALLOC::template VAL<char>;
	using value_type = int;

	//value_type* allocate(std::size_t n) {
	//	if(n == 1) return (value_type*)&ALLOC::construct()();
	//	else return new value_type[n];
	//}

	//void deallocate(value_type* p, std::size_t n) {
	//	if(n != 1) delete[] p;
	//}

};






}


