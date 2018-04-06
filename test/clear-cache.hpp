#pragma once

#include <vector>

inline void clear_cache() {
	std::vector<char> v(20'000'000); // 20MB
	for(int i=0; i<(int)v.size(); ++i) v[i] = i;
	long long result = 0;
	for(auto& e : v) result += e;
	//cout << "cache cleared " << result %2 << endl;
}

