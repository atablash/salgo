#include "common.hpp"
#include <benchmark/benchmark.h>

#include <salgo/memory-block>

#include <salgo/alloc/salgo-from-std-allocator>
#include <salgo/alloc/crude-allocator>
#include <salgo/alloc/random-allocator>
#include <salgo/alloc/array-allocator>

using namespace benchmark;

using namespace salgo;
using namespace salgo::alloc;






static void SEQUENTIAL_std(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	Alloc alloc;

	Dynamic_Array<Alloc::Handle> v;
	v.reserve( state.max_iterations );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc(e).destruct();
}
BENCHMARK( SEQUENTIAL_std )->MinTime(0.1);




static void SEQUENTIAL_salgo_crude(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	Alloc alloc;

	Dynamic_Array<Alloc::Handle> v;
	v.reserve( state.max_iterations );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc(e).destruct();
}
BENCHMARK( SEQUENTIAL_salgo_crude )->MinTime(0.1);




static void SEQUENTIAL_salgo_random(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	Alloc alloc;

	Dynamic_Array<Alloc::Handle> v;
	v.reserve( state.max_iterations );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc(e).destruct();
}
BENCHMARK( SEQUENTIAL_salgo_random )->MinTime(0.1);




static void SEQUENTIAL_salgo_vector(State& state) {
	srand(69); clear_cache();

	using Alloc = Array_Allocator<int>;
	Alloc alloc;

	Dynamic_Array<Alloc::Handle> v;
	v.reserve( state.max_iterations );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc(e).destruct();
}
BENCHMARK( SEQUENTIAL_salgo_vector )->MinTime(0.1);

















static void QUEUE_std(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.max_iterations/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.domain()+fst-lst)%v.domain() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.domain();
			alloc.destruct(h);
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.domain();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.domain();
		alloc.destruct(h);
	}
}
BENCHMARK( QUEUE_std )->MinTime(0.1);






static void QUEUE_salgo_crude(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.max_iterations/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.domain()+fst-lst)%v.domain() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.domain();
			alloc.destruct(h);
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.domain();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.domain();
		alloc.destruct(h);
	}
}
BENCHMARK( QUEUE_salgo_crude )->MinTime(0.1);






static void QUEUE_salgo_random(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.max_iterations/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.domain()+fst-lst)%v.domain() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.domain();
			alloc(h).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.domain();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.domain();
		alloc(h).destruct();
	}
}
BENCHMARK( QUEUE_salgo_random )->MinTime(0.1);






static void QUEUE_salgo_vector(State& state) {
	srand(69); clear_cache();

	using Alloc = Array_Allocator<int>;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.max_iterations/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.domain()+fst-lst)%v.domain() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.domain();
			alloc(h).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.domain();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.domain();
		alloc(h).destruct();
	}
}
BENCHMARK( QUEUE_salgo_vector )->MinTime(0.1);

















static void RANDOM_std(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::CONSTRUCTED_FLAGS v(state.max_iterations/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.domain();
		if(v(idx).is_constructed()) {
			alloc.destruct( v[idx] );
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc(e).destruct();
	}
}
BENCHMARK( RANDOM_std )->MinTime(0.1);






static void RANDOM_salgo_crude(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::CONSTRUCTED_FLAGS v(state.max_iterations/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.domain();
		if(v(idx).is_constructed()) {
			alloc.destruct( v[idx] );
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc(e).destruct();
	}
}
BENCHMARK( RANDOM_salgo_crude )->MinTime(0.1);






static void RANDOM_salgo_random(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::CONSTRUCTED_FLAGS v(state.max_iterations/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.domain();
		if(v(idx).is_constructed()) {
			alloc( v[idx] ).destruct();
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc(e).destruct();
	}
}
BENCHMARK( RANDOM_salgo_random )->MinTime(0.1);






static void RANDOM_salgo_vector(State& state) {
	srand(69); clear_cache();

	using Alloc = Array_Allocator<int>;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::CONSTRUCTED_FLAGS v(state.max_iterations/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.domain();
		if(v(idx).is_constructed()) {
			alloc( v[idx] ).destruct();
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc(e).destruct();
	}
}
BENCHMARK( RANDOM_salgo_vector )->MinTime(0.1);







BENCHMARK_MAIN();


