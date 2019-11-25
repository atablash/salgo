#include "common.hpp"
#include <benchmark/benchmark.h>

#include <salgo/list>
#include <salgo/salgo-from-std-allocator>
#include <salgo/crude-allocator>
#include <salgo/random-allocator>
#include <salgo/vector-allocator>

#include <list>

using namespace benchmark;

using namespace salgo;













static void INSERT_ERASE_std(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;
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
				li.emplace_front( rand() );
			}

			it = ne;
			++ith;
		}
	}
}
BENCHMARK( INSERT_ERASE_std )->MinTime(0.1);



static void INSERT_ERASE_salgo_stdalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	salgo::List<int> :: COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.count()) ) {
		int ith = 0;
		for(auto& e : li) {

			if(ith%2) {
				e.erase();
				li.emplace_front( rand() );
			}

			++ith;
		}
	}
}
BENCHMARK( INSERT_ERASE_salgo_stdalloc )->MinTime(0.1);





static void INSERT_ERASE_salgo_crudealloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

	using Alloc = salgo::Crude_Allocator<int>;
	salgo::List<int> :: COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.count()) ) {
		int ith = 0;
		for(auto& e : li) {

			if(ith%2) {
				e.erase();
				li.emplace_front( rand() );
			}

			++ith;
		}
	}
}
BENCHMARK( INSERT_ERASE_salgo_crudealloc )->MinTime(0.1);





static void INSERT_ERASE_salgo_randalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

	using Alloc = salgo::Random_Allocator<int>;
	salgo::List<int> :: COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.count()) ) {
		int ith = 0;
		for(auto& e : li) {

			if(ith%2) {
				e.erase();
				li.emplace_front( rand() );
			}

			++ith;
		}
	}
}
BENCHMARK( INSERT_ERASE_salgo_randalloc )->MinTime(0.1);




static void INSERT_ERASE_salgo_vectoralloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

	using Alloc = salgo::Vector_Allocator<int>;
	salgo::List<int> :: COUNTABLE ::ALLOCATOR<Alloc> li;

	for(int i=0; i<N; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	while( state.KeepRunningBatch(li.count()) ) {
		int ith = 0;
		for(auto& e : li) {

			if(ith%2) {
				e.erase();
				li.emplace_front( rand() );
			}

			++ith;
		}
	}
}
BENCHMARK( INSERT_ERASE_salgo_vectoralloc )->MinTime(0.1);



















static void ITERATE_std(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;
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
BENCHMARK( ITERATE_std )->MinTime(0.1);






static void ITERATE_salgo_countable_stdalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

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
BENCHMARK( ITERATE_salgo_countable_stdalloc )->MinTime(0.1);





static void ITERATE_salgo_countable_crudealloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

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
BENCHMARK( ITERATE_salgo_countable_crudealloc )->MinTime(0.1);





static void ITERATE_salgo_countable_randalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

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
BENCHMARK( ITERATE_salgo_countable_randalloc )->MinTime(0.1);






static void ITERATE_salgo_countable_vectoralloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

	using Alloc = salgo::Vector_Allocator<int>;
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
BENCHMARK( ITERATE_salgo_countable_vectoralloc )->MinTime(0.1);

















static void INSERT_std(State& state) {
	srand(69); clear_cache();

	std::list<int> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_std )->MinTime(0.1);





static void INSERT_salgo_countable_stdalloc(State& state) {
	srand(69); clear_cache();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo_countable_stdalloc )->MinTime(0.1);



static void INSERT_salgo_countable_crudealloc(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Crude_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo_countable_crudealloc )->MinTime(0.1);



static void INSERT_salgo_countable_randalloc(State& state) {
	srand(69); clear_cache();

	using Alloc = salgo::Random_Allocator<int>;
	salgo::List<int> ::COUNTABLE ::ALLOCATOR<Alloc> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo_countable_randalloc )->MinTime(0.1);




static void INSERT_salgo(State& state) {
	srand(69); clear_cache();

	salgo::List<int> li;

	for(auto _ : state) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}
}
BENCHMARK( INSERT_salgo )->MinTime(0.1);


















static void ERASE_std(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;
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
BENCHMARK( ERASE_std )->MinTime(0.1);




static void ERASE_salgo_countable_stdalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

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
BENCHMARK( ERASE_salgo_countable_stdalloc )->MinTime(0.1);






static void ERASE_salgo_countable_crudealloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

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
BENCHMARK( ERASE_salgo_countable_crudealloc )->MinTime(0.1);




static void ERASE_salgo_countable_randalloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

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
BENCHMARK( ERASE_salgo_countable_randalloc )->MinTime(0.1);




static void ERASE_salgo_countable_vectoralloc(State& state) {
	srand(69); clear_cache();

	const int N = state.max_iterations;

	using Alloc = salgo::Vector_Allocator<int>;
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
BENCHMARK( ERASE_salgo_countable_vectoralloc )->MinTime(0.1);
















BENCHMARK_MAIN();






