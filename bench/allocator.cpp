#include "common.hpp"
#include <benchmark/benchmark.h>

#include <salgo/memory-block.hpp>

#include <salgo/salgo-from-std-allocator.hpp>
#include <salgo/crude-allocator.hpp>
#include <salgo/random-allocator.hpp>
#include <salgo/vector-allocator.hpp>

using namespace benchmark;

using namespace salgo;






static void SEQUENTIAL_std(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	Alloc alloc;

	Vector<Alloc::Handle> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc.destruct(e);
}
BENCHMARK( SEQUENTIAL_std );





static void SEQUENTIAL_salgo_crude(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	Alloc alloc;

	Vector<Alloc::Handle> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc.destruct(e);
}
BENCHMARK( SEQUENTIAL_salgo_crude );




static void SEQUENTIAL_salgo_random(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	Alloc alloc;

	Vector<Alloc::Handle> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc.destruct(e);
}
BENCHMARK( SEQUENTIAL_salgo_random );




static void SEQUENTIAL_salgo_vector(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Vector_Allocator<int>;
	Alloc alloc;

	Vector<Alloc::Handle> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		auto h = alloc.construct().handle();
		v.emplace_back( h );
	}

	for(auto& e : v) alloc.destruct(e);
}
BENCHMARK( SEQUENTIAL_salgo_vector );

















static void QUEUE_std(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.iterations()/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.size()+fst-lst)%v.size() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.size();
			alloc.destruct(h);
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.size();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.size();
		alloc.destruct(h);
	}
}
BENCHMARK( QUEUE_std );






static void QUEUE_salgo_crude(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.iterations()/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.size()+fst-lst)%v.size() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.size();
			alloc.destruct(h);
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.size();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.size();
		alloc.destruct(h);
	}
}
BENCHMARK( QUEUE_salgo_crude );






static void QUEUE_salgo_random(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.iterations()/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.size()+fst-lst)%v.size() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.size();
			alloc.destruct(h);
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.size();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.size();
		alloc.destruct(h);
	}
}
BENCHMARK( QUEUE_salgo_random );






static void QUEUE_salgo_vector(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Vector_Allocator<int>;
	Alloc alloc;

	// crude queue implementation
	Memory_Block<Alloc::Handle> ::DENSE v(state.iterations()/10 + 10);
	int fst = 0;
	int lst = 0;
	auto is_empty = [&](){ return fst == lst; };
	auto is_full  = [&](){ return fst != lst && (v.size()+fst-lst)%v.size() <= 2; };

	for(auto _ : state) {
		if(is_full() || (!is_empty() && rand()%2)) {
			auto h = v[fst];
			fst = (fst+1)%v.size();
			alloc.destruct(h);
		}
		else {
			auto h = alloc.construct().handle();
			v[lst] = h;
			lst = (lst+1)%v.size();
		}
	}

	while(!is_empty()) {
		auto h = v[fst];
		fst = (fst+1)%v.size();
		alloc.destruct(h);
	}
}
BENCHMARK( QUEUE_salgo_vector );

















static void RANDOM_std(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::EXISTS v(state.iterations()/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.size();
		if(v(idx).exists()) {
			alloc.destruct( v[idx] );
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc.destruct(e);
	}
}
BENCHMARK( RANDOM_std );






static void RANDOM_salgo_crude(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::EXISTS v(state.iterations()/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.size();
		if(v(idx).exists()) {
			alloc.destruct( v[idx] );
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc.destruct(e);
	}
}
BENCHMARK( RANDOM_salgo_crude );






static void RANDOM_salgo_random(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::EXISTS v(state.iterations()/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.size();
		if(v(idx).exists()) {
			alloc.destruct( v[idx] );
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc.destruct(e);
	}
}
BENCHMARK( RANDOM_salgo_random );






static void RANDOM_salgo_vector(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Vector_Allocator<int>;
	Alloc alloc;

	Memory_Block<Alloc::Handle> ::EXISTS v(state.iterations()/10 + 10);

	for(auto _ : state) {
		int idx = rand() % v.size();
		if(v(idx).exists()) {
			alloc.destruct( v[idx] );
			v(idx).destruct();
		}
		else {
			auto h = alloc.construct().handle();
			v(idx).construct(h);
		}
	}

	for(auto& e : v) {
		alloc.destruct(e);
	}
}
BENCHMARK( RANDOM_salgo_vector );







BENCHMARK_MAIN();


