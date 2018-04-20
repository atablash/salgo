#include "common.hpp"
#include <benchmark/benchmark.h>

#include <salgo/vector.hpp>
#include <salgo/chunked-vector.hpp>
#include <salgo/unordered-vector.hpp>
#include <salgo/reversed.hpp>

#include <vector>

using namespace benchmark;

using namespace salgo;






static void PUSH_BACK_std(State& state) {
	srand(69); clear_cache();

	std::vector<int> v;

	for(auto _ : state) {
		v.emplace_back( rand() );
	}

	int sum = 0;
	for(auto& e : v) sum += e;
	DoNotOptimize(sum);
}
BENCHMARK( PUSH_BACK_std );





static void PUSH_BACK_salgo(State& state) {
	srand(69); clear_cache();

	salgo::Vector<int> v;

	for(auto _ : state) {
		v.emplace_back( rand() );
	}

	int sum = 0;
	for(auto& e : v) sum += e;
	DoNotOptimize(sum);
}
BENCHMARK( PUSH_BACK_salgo );




static void PUSH_BACK_salgo_chunked(State& state) {
	srand(69); clear_cache();

	salgo::Chunked_Vector<int> v;

	for(auto _ : state) {
		v.emplace_back( rand() );
	}

	int sum = 0;
	for(auto& e : v) sum += e;
	DoNotOptimize(sum);
}
BENCHMARK( PUSH_BACK_salgo_chunked );











static void PUSH_BACK_reserved_std(State& state) {
	srand(69); clear_cache();

	std::vector<int> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		v.emplace_back( rand() );
	}

	int sum = 0;
	for(auto& e : v) sum += e;
	DoNotOptimize(sum);
}
BENCHMARK( PUSH_BACK_reserved_std );





static void PUSH_BACK_reserved_salgo(State& state) {
	srand(69); clear_cache();

	salgo::Vector<int> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		v.emplace_back( rand() );
	}

	int sum = 0;
	for(auto& e : v) sum += e;
	DoNotOptimize(sum);
}
BENCHMARK( PUSH_BACK_reserved_salgo );


static void PUSH_BACK_reserved_salgo_chunked(State& state) {
	srand(69); clear_cache();

	salgo::Chunked_Vector<int> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		v.emplace_back( rand() );
	}

	int sum = 0;
	for(auto& e : v) sum += e;
	DoNotOptimize(sum);
}
BENCHMARK( PUSH_BACK_reserved_salgo_chunked );










static void RANDOM_ACCESS_std(State& state) {
	srand(69); clear_cache();

	std::vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	for(auto _ : state) {
		sum += v[rand()%v.size()];
	}
	DoNotOptimize(sum);
}
BENCHMARK( RANDOM_ACCESS_std );



static void RANDOM_ACCESS_salgo(State& state) {
	srand(69); clear_cache();

	salgo::Vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	for(auto _ : state) {
		sum += v[rand()%v.size()];
	}
	DoNotOptimize(sum);
}
BENCHMARK( RANDOM_ACCESS_salgo );



static void RANDOM_ACCESS_salgo_chunked(State& state) {
	srand(69); clear_cache();

	salgo::Chunked_Vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	for(auto _ : state) {
		sum += v[rand()%v.size()];
	}
	DoNotOptimize(sum);
}
BENCHMARK( RANDOM_ACCESS_salgo_chunked );












static void SEQUENTIAL_ACCESS_std(State& state) {
	srand(69); clear_cache();

	std::vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(int i=0; i<(int)v.size(); ++i) sum += v[i];
		DoNotOptimize(sum);
	}
}
BENCHMARK( SEQUENTIAL_ACCESS_std )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);


static void SEQUENTIAL_ACCESS_salgo(State& state) {
	srand(69); clear_cache();

	salgo::Vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(int i=0; i<v.size(); ++i) sum += v[i];
		DoNotOptimize(sum);
	}
}
BENCHMARK( SEQUENTIAL_ACCESS_salgo )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);


static void SEQUENTIAL_ACCESS_salgo_chunked(State& state) {
	srand(69); clear_cache();

	salgo::Chunked_Vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(int i=0; i<v.size(); ++i) sum += v[i];
		DoNotOptimize(sum);
	}
}
BENCHMARK( SEQUENTIAL_ACCESS_salgo_chunked )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);












static void FOREACH_ACCESS_std(State& state) {
	srand(69); clear_cache();

	std::vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(auto& e : v) sum += e;
		DoNotOptimize(sum);
	}
}
BENCHMARK( FOREACH_ACCESS_std )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);


static void FOREACH_ACCESS_salgo(State& state) {
	srand(69); clear_cache();

	salgo::Vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(auto& e : v) sum += e;
		DoNotOptimize(sum);
	}
}
BENCHMARK( FOREACH_ACCESS_salgo )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);


static void FOREACH_ACCESS_salgo_unordered(State& state) {
	srand(69); clear_cache();

	salgo::Unordered_Vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(auto& e : v) sum += e;
		DoNotOptimize(sum);
	}
}
BENCHMARK( FOREACH_ACCESS_salgo_unordered )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);


static void FOREACH_ACCESS_salgo_chunked(State& state) {
	srand(69); clear_cache();

	salgo::Chunked_Vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(auto& e : v) sum += e;
		DoNotOptimize(sum);
	}
}
BENCHMARK( FOREACH_ACCESS_salgo_chunked )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);









static void FOREACH_ACCESS_REV_salgo_unordered(State& state) {
	srand(69); clear_cache();

	salgo::Unordered_Vector<int> v( state.range(0) );
	for(auto& e : v) e = rand();

	for(auto _ : state) {
		int sum = 0;
		for(auto& e : Reversed(v)) sum += e;
		DoNotOptimize(sum);
	}
}
BENCHMARK( FOREACH_ACCESS_REV_salgo_unordered )->Arg(1'000'000)->Unit(benchmark::kMicrosecond);











BENCHMARK_MAIN();


