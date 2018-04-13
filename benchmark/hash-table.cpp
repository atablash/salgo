#include <benchmark/benchmark.h>

#include <salgo/hash-table.hpp>

#include <unordered_set>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;

using namespace benchmark;

using namespace salgo;





static void REHASH_std(State& state) {
	const int size = state.range(0);

	srand(0);

	for(auto _ : state) {
		state.PauseTiming();

		{
			std::unordered_multiset<int> s;
			s.rehash(size);
			for(int i=0; i<size; ++i) s.emplace( rand() );

			state.ResumeTiming();
			s.rehash( state.range(1) );
			state.PauseTiming();

			for(auto e : s) DoNotOptimize(e);
		}

		state.ResumeTiming();
	}
}
BENCHMARK( REHASH_std )->Args({1'056'323, 1'142'821})->Unit(benchmark::kMillisecond);





static void REHASH_salgo(State& state) {
	const int size = state.range(0);

	srand(0);

	for(auto _ : state) {
		state.PauseTiming();

		{
			salgo::Hash_Table<int> s;
			s.rehash(size);
			for(int i=0; i<size; ++i) s.emplace( rand() );

			// std::cout << "max_bucket_size: " << s.max_bucket_size() << std::endl;

			state.ResumeTiming();
			s.rehash( state.range(1) );
			state.PauseTiming();

			for(auto e : s) DoNotOptimize(e());
		}

		state.ResumeTiming();
	}
}
BENCHMARK( REHASH_salgo )->Args({1'056'323, 1'142'821})->Unit(benchmark::kMillisecond);
















static void INSERT__std(State& state) {
	srand(69);

	std::unordered_multiset<int> s;

	for(auto _ : state) {
		s.emplace( rand() );
	}
}
BENCHMARK( INSERT__std );





static void INSERT__salgo(State& state) {
	srand(69);

	Hash_Table<int> s;

	for(auto _ : state) {
		s.emplace( rand() );
	}
}
BENCHMARK( INSERT__salgo );















static void RESERVED_INSERT__std(State& state) {
	const int N = state.iterations();

	srand(69);

	std::unordered_multiset<int> s;
	s.rehash( N + 10 );

	for(auto _ : state) {
		s.emplace( rand() );
	}
}
BENCHMARK( RESERVED_INSERT__std );





static void RESERVED_INSERT__salgo(State& state) {
	const int N = state.iterations();

	srand(69);

	Hash_Table<int> s;
	s.rehash( N + 10 );

	for(auto _ : state) {
		s.emplace( rand() );
	}
}
BENCHMARK( RESERVED_INSERT__salgo );










static void FIND__std(State& state) {
	const int N = state.iterations();

	srand(69);

	std::unordered_multiset<int> s;
	s.rehash( N + 10 );

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(auto _ : state) {
		auto iter = s.find( rand() % N );
		if(iter != s.end()) DoNotOptimize(*iter);
	}
}
BENCHMARK( FIND__std );





static void FIND__salgo(State& state) {
	const int N = state.iterations();

	srand(69);

	salgo::Hash_Table<int> s;
	s.rehash( N + 10 );

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(auto _ : state) {
		auto acc = s( rand() % N );
		if(acc.exists()) DoNotOptimize( acc() );
	}
}
BENCHMARK( FIND__salgo );










static void FIND_ERASE__std(State& state) {
	const int N = state.iterations();

	srand(69);

	std::unordered_multiset<int> s;
	s.rehash( N + 10 );

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(auto _ : state) {
		auto iter = s.find( rand() % N );
		if(iter != s.end()) s.erase( iter );
	}
}
BENCHMARK( FIND_ERASE__std );





static void FIND_ERASE__salgo(State& state) {
	const int N = state.iterations();

	srand(69);

	salgo::Hash_Table<int> s;
	s.rehash( N + 10 );

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(auto _ : state) {
		auto e = s( rand() % N );
		if(e.exists()) e.erase();
	}
}
BENCHMARK( FIND_ERASE__salgo );













static void ITERATE__std(State& state) {
	const int N = state.iterations();

	srand(69);

	std::unordered_multiset<int> s;
	s.rehash( N + 10 );

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(auto _ : state) {
		long long result = 0;
		for(auto e : s) result += e;
		DoNotOptimize(result);
	}
}
BENCHMARK( ITERATE__std );



static void ITERATE__salgo(State& state) {
	const int N = state.iterations();

	srand(69);

	salgo::Hash_Table<int> s;
	s.rehash( N + 10 );

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(auto _ : state) {
		long long result = 0;
		for(auto e : s) result += e;
		DoNotOptimize(result);
	}
}
BENCHMARK( ITERATE__salgo );















/*
static double run_unordered_set_alloc(int N) {
	//clear_cache();

	auto t0 = steady_clock::now();

	fast_srand(69);

	using Alloc = Allocator<int> :: SINGLETON;
	using Std_Alloc = Std_From_Salgo_Allocator< Alloc >;

	std::unordered_multiset<int, std::hash<int>, std::equal_to<int>, Std_Alloc> s;

	s.reserve(N);

	//cout << "max_load_factor: " << s.max_load_factor() << endl;

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(int i=0; i<N; ++i) {
		auto iter = s.find( rand() % N );
		if(iter != s.end()) s.erase( iter );
	}

	long long result = 0;
	for(auto e : s) result += e;

	duration<double> diff = steady_clock::now() - t0;
	return diff.count();
}
*/










BENCHMARK_MAIN();









/* TODO


static double run_hash_table_malloc(int N) {
	//clear_cache();

	auto t0 = steady_clock::now();

	fast_srand(69);

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;

	Hash_Table<int> :: ALLOCATOR<Alloc> s;

	s.rehash(N);

	for(int i=0; i<N; ++i) {
		s.emplace( rand() % N );
	}

	for(int i=0; i<N; ++i) {
		auto e = s( rand() % N );
		if(e.exists()) e.erase();
	}

	long long result = 0;
	for(auto e : s) result += e;

	duration<double> diff = steady_clock::now() - t0;
	return diff.count();
}



	
*/

