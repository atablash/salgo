#include <salgo/sparse-vector.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;






TEST(Sparse_vector, test_push_delete_compact) {

	Sparse_Vector<int>::COUNTABLE m;
	m.emplace_back(1); //
	m.emplace_back(2);
	m.emplace_back(3); //
	m.emplace_back(4);
	m.emplace_back(5); //

	EXPECT_EQ(5, m.count());

	//EXPECT_EQ(0, m.domain_begin());
	//EXPECT_EQ(5, m.domain_end());

	EXPECT_EQ(5, m.domain());

	EXPECT_EQ(1, m[0].val());
	EXPECT_EQ(2, m[1].val());
	EXPECT_EQ(3, m[2].val());
	EXPECT_EQ(4, m[3].val());
	EXPECT_EQ(5, m[4].val());

	{
		int sum = 0;
		for(const auto& e : m) sum += e.val();
		EXPECT_EQ(15, sum);
	}

	m[0].erase();
	m[2].erase();
	m[4].erase();

	EXPECT_TRUE( m[1].exists());
	EXPECT_FALSE(m[2].exists());

	{
		int sum = 0;
		for(const auto& e : m) sum += e.val();
		EXPECT_EQ(6, sum);
	}

	{
		vector<pair<int,int>> remap;
		m.compact([&](int fr, int to){ remap.emplace_back(fr,to); });
		EXPECT_EQ( decltype(remap)({{1,0}, {3,1}}), remap );
	}

	{
		int sum = 0;
		for(const auto& e : m) sum += e.val();
		EXPECT_EQ(6, sum);
	}
}
















void run_vector(int N, int type) {

	cout << "vector:\t";
	srand(69);
	auto t0 = steady_clock::now();

	vector<double> v(N);
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		v[ii] = rand();
	}

	long long result = 0;
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		result += v[ii];
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}



void run_vector_inplace(int N, int type) {

	cout << "vector, in-place exists flag:\t";
	srand(69);
	auto t0 = steady_clock::now();

	struct Node {
		bool exists = true;
		double val = 0.0;
	};

	vector<Node> v(N);
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		if(v[ii].exists) {
			v[ii].val = rand();
			if(rand()%2) v[ii].exists = false;
		}
	}

	long long result = 0;
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		if(v[ii].exists) result += v[ii].val;
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}


void run_vector_external(int N, int type) {
	cout << "vector, external exists array:\t";
	srand(69);
	auto t0 = steady_clock::now();

	vector<char> exists(N,true);
	vector<double> v(N);
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		if(exists[ii]) {
			v[ii] = rand();
			if(rand()%2) exists[ii] = false;
		}
	}

	long long result = 0;
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		if(exists[ii]) result += v[ii];
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}


void run_vector_external_bitset(int N, int type) {

	cout << "vector, external exists bitset:\t";
	srand(69);
	auto t0 = steady_clock::now();

	vector<bool> exists(N,true);
	vector<double> v(N);
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		if(exists[ii]) {
			v[ii] = rand();
			if(rand()%2) exists[ii] = false;
		}
	}

	long long result = 0;
	for(int i=0; i<N; ++i) {
		int ii = i;
		if(type == 1) ii = rand() % N;

		if(exists[ii]) result += v[ii];
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}

void run_sparse_vector(int N, int type) {

	cout << "Sparse_Vector:\t";
	srand(69);
	auto t0 = steady_clock::now();

	Sparse_Vector<double> v(N);
	long long result = 0;

	if(type == 0) {
		// sequential
		for(auto e : v) {
			e.val() = rand();
			if(rand()%2) e.erase();
		}

		for(const auto& e : v) {
			result += e.val();
		}
	}
	else {
		// random access
		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v[ii].exists()) {
				v[ii].val() = rand();
				if(rand()%2) v[ii].erase();
			}
		}

		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v[ii].exists()) result += v[ii].val();
		}
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}






void run_sparse_vector_index(int N, int type) {

	cout << "Sparse_Vector index:\t";
	srand(69);
	auto t0 = steady_clock::now();

	Sparse_Vector<double> v(N);
	long long result = 0;

	if(type == 0) {
		// sequential
		for(int i=0; i<N; ++i) {
			v[i].val() = rand();
			if(rand()%2) v[i].erase();
		}

		for(int i=0; i<N; ++i) {
			if(v[i].exists()) result += v[i].val();
		}
	}
	else {
		// random access
		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v[ii].exists()) {
				v[ii].val() = rand();
				if(rand()%2) v[ii].erase();
			}
		}

		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v[ii].exists()) result += v[ii].val();
		}
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}




void run_sparse_vector_noacc(int N, int type) {

	cout << "Sparse_Vector index, noacc:\t";
	srand(69);
	auto t0 = steady_clock::now();

	Sparse_Vector<double> v(N);
	long long result = 0;

	if(type == 0) {
		// sequential
		for(int i=0; i<N; ++i) {
			v.at(i) = rand();
			if(rand()%2) v.erase(i);
		}

		for(int i=0; i<N; ++i) {
			if(v.exists(i)) result += v.at(i);
		}
	}
	else {
		// random access
		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v.exists(ii)) {
				v.at(ii) = rand();
				if(rand()%2) v.erase(ii);
			}
		}

		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v.exists(ii)) result += v.at(ii);
		}
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}









TEST(Performance, sparse_vector) {
	const int N = 1'000'000;

	for(int type=0; type<2; ++type) {

		cout << endl << (type==0 ? "SEQUENTIAL" : "RANDOM ACCESS") << endl;

		run_vector(N, type);
		run_vector_inplace(N,type);
		run_vector_external(N,type);
		run_vector_external_bitset(N,type);
		run_sparse_vector(N,type);
		run_sparse_vector_index(N,type);
		run_sparse_vector_noacc(N,type);
	}
}




/*

// simple version - g++-7

SEQUENTIAL
vector:	10738140233948518, 0.219789
vector, in-place exists flag:	5367851528401683, 0.235499
vector, external exists bitset:	5367851528401683, 0.269546
vector, external exists array:	5367851528401683, 0.215052
Sparse_Vector:	5370846108780908, 0.254665

RANDOM ACCESS
vector:	6787011535458750, 0.474145
vector, in-place exists flag:	3397131765529901, 0.678932
vector, external exists bitset:	3397131765529901, 0.638737
vector, external exists array:	3397131765529901, 0.684499
Sparse_Vector:	2556795968057347, 0.682569





///// accessor version - g++-7

SEQUENTIAL
vector:	10738140233948518, 0.211834
vector, in-place exists flag:	5367851528401683, 0.225002
vector, external exists array:	5367851528401683, 0.212957
vector, external exists bitset:	5367851528401683, 0.264526
Sparse_Vector:	5370846108780908, 0.25875
Sparse_Vector index:	5370846108780908, 0.24794

RANDOM ACCESS
vector:	6787011535458750, 0.433005
vector, in-place exists flag:	3397131765529901, 0.632824
vector, external exists array:	3397131765529901, 0.734075
vector, external exists bitset:	3397131765529901, 0.618504
Sparse_Vector:	2556795968057347, 0.766812					// slow
Sparse_Vector index:	2556795968057347, 0.781151			// slow





// accessor version - clang++-5.0

SEQUENTIAL
vector:	10738140233948518, 0.213025
vector, in-place exists flag:	5367851528401683, 0.221979
vector, external exists array:	5367851528401683, 0.238649
vector, external exists bitset:	5367851528401683, 0.274027
Sparse_Vector:	5370846108780908, 0.25938
Sparse_Vector index:	5370846108780908, 0.244503

RANDOM ACCESS
vector:	6787011535458750, 0.44723
vector, in-place exists flag:	3397131765529901, 0.66427
vector, external exists array:	3397131765529901, 0.67194
vector, external exists bitset:	3397131765529901, 0.620741
Sparse_Vector:	2556795968057347, 0.68991
Sparse_Vector index:	2556795968057347, 0.687364


*/




