#include <salgo/hash-table.hpp>
#include "fast-rand.hpp"
#include "clear-cache.hpp"

#include <chrono>
#include <unordered_set>

using namespace std;
using namespace salgo;
using namespace std::chrono;


#include <gtest/gtest.h>


DECLARE_int32(perf);


using namespace salgo;

TEST(Hash_Table, iterate) {
	Hash_Table<int> ht;
	ht.emplace(3);
	ht.emplace(30);
	ht.emplace(300);
	ht.emplace(3000);
	ht.emplace(30000);
	ht.emplace(300000);

	int sum = 0;
	for(auto e : ht) {
		sum += e;
	}
	EXPECT_EQ(333333, sum);
}

TEST(Hash_Table, erase) {
	Hash_Table<int> ht;
	ht.emplace(3);
	ht.emplace(30);
	ht.emplace(300);
	ht.emplace(3000);
	ht.emplace(30000);
	ht.emplace(300000);

	ht(30).erase();
	ht(3000).erase();
	ht(300000).erase();

	int sum = 0;
	for(auto e : ht) {
		sum += e;
	}

	EXPECT_EQ(30303, sum);
}





static void run_unordered_set(int N) {
	clear_cache();

	auto t0 = steady_clock::now();

	fast_srand(69);

	std::unordered_multiset<int> s;

	for(int i=0; i<N; ++i) {
		s.emplace( fast_rand() % N );
	}

	for(int i=0; i<N; ++i) {
		auto iter = s.find( fast_rand() % N );
		if(iter != s.end()) s.erase( iter );
	}

	long long result = 0;
	for(auto e : s) result += e;

	duration<double> diff = steady_clock::now() - t0;
	cout << result%100 << ", " << diff.count() << endl;
}


static void run_hash_table(int N) {
	clear_cache();

	auto t0 = steady_clock::now();

	fast_srand(69);

	Hash_Table<int> s;

	for(int i=0; i<N; ++i) {
		s.emplace( fast_rand() % N );
	}

	for(int i=0; i<N; ++i) {
		auto e = s( fast_rand() % N );
		if(e.exists()) e.erase();
	}

	long long result = 0;
	for(auto e : s) result += e;

	duration<double> diff = steady_clock::now() - t0;
	cout << result%100 << ", " << diff.count() << endl;
}



TEST(Hash_Table, perf) {
	int N = FLAGS_perf;
	run_unordered_set(N);
	run_hash_table(N);
}

