#include <benchmark/benchmark.h>

#include <salgo/vector.hpp>
#include <salgo/chunked-vector.hpp>

#include <vector>

using namespace benchmark;

using namespace salgo;






static void PUSH_BACK_std(State& state) {
	srand(69);

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
	srand(69);

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
	srand(69);

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
	srand(69);

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
	srand(69);

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
	srand(69);

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
	srand(69);

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
	srand(69);

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
	srand(69);

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
	srand(69);

	std::vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	int i=0;
	for(auto _ : state) {
		sum += v[i++];
	}
	DoNotOptimize(sum);
}
BENCHMARK( SEQUENTIAL_ACCESS_std );


static void SEQUENTIAL_ACCESS_salgo(State& state) {
	srand(69);

	salgo::Vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	int i=0;
	for(auto _ : state) {
		sum += v[i++];
	}
	DoNotOptimize(sum);
}
BENCHMARK( SEQUENTIAL_ACCESS_salgo );


static void SEQUENTIAL_ACCESS_salgo_chunked(State& state) {
	srand(69);

	salgo::Chunked_Vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	int i=0;
	for(auto _ : state) {
		sum += v[i++];
	}
	DoNotOptimize(sum);
}
BENCHMARK( SEQUENTIAL_ACCESS_salgo_chunked );












static void FOREACH_ACCESS_std(State& state) {
	srand(69);

	std::vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	while( state.KeepRunningBatch(v.size()) ) {
		for(auto& e : v) sum += e;
	}
	DoNotOptimize(sum);
}
BENCHMARK( FOREACH_ACCESS_std );


static void FOREACH_ACCESS_salgo(State& state) {
	srand(69);

	salgo::Vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	while( state.KeepRunningBatch(v.size()) ) {
		for(auto& e : v) sum += e;
	}
	DoNotOptimize(sum);
}
BENCHMARK( FOREACH_ACCESS_salgo );


static void FOREACH_ACCESS_salgo_chunked(State& state) {
	srand(69);

	salgo::Chunked_Vector<int> v( state.iterations() );
	for(auto& e : v) e = rand();

	int sum = 0;
	while( state.KeepRunningBatch(v.size()) ) {
		for(auto& e : v) sum += e;
	}
	DoNotOptimize(sum);
}
BENCHMARK( FOREACH_ACCESS_salgo_chunked );











BENCHMARK_MAIN();


