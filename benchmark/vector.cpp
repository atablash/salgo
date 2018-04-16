#include <benchmark/benchmark.h>

#include <salgo/vector.hpp>

#include <vector>

using namespace benchmark;

using namespace salgo;






static void PUSH_BACK_std(State& state) {
	srand(69);

	std::vector<int> v;

	for(auto _ : state) {
		v.emplace_back( rand() );
		DoNotOptimize( &v.back() );
	}
}
BENCHMARK( PUSH_BACK_std );





static void PUSH_BACK_salgo(State& state) {
	srand(69);

	salgo::Vector<int> v;

	for(auto _ : state) {
		v.emplace_back( rand() );
		DoNotOptimize( &v.back()() );
	}
}
BENCHMARK( PUSH_BACK_salgo );









static void PUSH_BACK_reserved_std(State& state) {
	srand(69);

	std::vector<int> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		v.emplace_back( rand() );
		DoNotOptimize( &v.back() );
	}
}
BENCHMARK( PUSH_BACK_reserved_std );





static void PUSH_BACK_reserved_salgo(State& state) {
	srand(69);

	salgo::Vector<int> v;
	v.reserve( state.iterations() );

	for(auto _ : state) {
		v.emplace_back( rand() );
		DoNotOptimize( &v.back()() );
	}
}
BENCHMARK( PUSH_BACK_reserved_salgo );










BENCHMARK_MAIN();


