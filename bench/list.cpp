#include "common.hpp"
#include <benchmark/benchmark.h>

#include <salgo/list.hpp>
#include <salgo/salgo-from-std-allocator.hpp>
#include <salgo/crude-allocator.hpp>
#include <salgo/random-allocator.hpp>

#include <list>

using namespace benchmark;

using namespace salgo;













static void ITERATE_std(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();
	std::list<int> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch( li.size() ) ) {
		int sum = 0;
		for(auto& e : li) sum += e;
		DoNotOptimize(sum);
	}



}
BENCHMARK( ITERATE_std );






static void ITERATE_salgo_countable_stdalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	salgo::List<int> :: COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch( li.count() ) ) {
		int sum = 0;
		for(auto& e : li) sum += e;
		DoNotOptimize(sum);
	}



}
BENCHMARK( ITERATE_salgo_countable_stdalloc );





static void ITERATE_salgo_countable_crudealloc(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();

	using Alloc = salgo::Crude_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch( li.count() ) ) {
		int sum = 0;
		for(auto& e : li) sum += e;
		DoNotOptimize(sum);
	}



}
BENCHMARK( ITERATE_salgo_countable_crudealloc );





static void ITERATE_salgo_countable_randalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();

	using Alloc = salgo::Random_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch( li.count() ) ) {
		int sum = 0;
		for(auto& e : li) sum += e;
		DoNotOptimize(sum);
	}



}
BENCHMARK( ITERATE_salgo_countable_randalloc );


















static void INSERT_std(State& state) {
	srand(69); clear_cache();

	std::list<int> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_std );





static void INSERT_salgo_countable_stdalloc(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo_countable_stdalloc );



static void INSERT_salgo_countable_crudealloc(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo_countable_crudealloc );



static void INSERT_salgo_countable_randalloc(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo_countable_randalloc );




static void INSERT_salgo(State& state) {
	srand(69); clear_cache();

	salgo::List<int> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo );


















static void ERASE_std(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();
	std::list<int> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.size()) ) {
		int ith = 0;
		for(auto it = li.begin(); it != li.end(); ++it) {
			auto ne = std::next(it);

			if(ith%2) {
				li.erase(it);
			}

			it = ne;
			++ith;
		}
	}
}
BENCHMARK( ERASE_std );




static void ERASE_salgo_countable_stdalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.count()) ) {
		int ith = 0;
		for(auto& e : li) {
			if(ith%2) {
				e.erase();
			}

			++ith;
		}
	}
}
BENCHMARK( ERASE_salgo_countable_stdalloc );






static void ERASE_salgo_countable_crudealloc(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();

	using Alloc = salgo::Crude_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.count()) ) {
		int ith = 0;
		for(auto& e : li) {
			if(ith%2) {
				e.erase();
			}

			++ith;
		}
	}
}
BENCHMARK( ERASE_salgo_countable_crudealloc );




static void ERASE_salgo_countable_randalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.iterations();

	using Alloc = salgo::Random_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.count()) ) {
		int ith = 0;
		for(auto& e : li) {
			if(ith%2) {
				e.erase();
			}

			++ith;
		}
	}
}
BENCHMARK( ERASE_salgo_countable_randalloc );








BENCHMARK_MAIN();






