#pragma once

#include <vector>
#include <cstdlib>
#include <benchmark/benchmark.h>

namespace {

inline void clear_cache() {
	std::vector<int> v(20'000'000 / sizeof(int)); // 20MB
	for(int i=0; i<(int)v.size(); ++i) v[i] = std::rand();
	long long result = 0;
	for(auto& e : v) {
		result += e;
	}
	//std::cout << "cache cleared " << result %2 << std::endl;
	benchmark::DoNotOptimize(result);
}


}


