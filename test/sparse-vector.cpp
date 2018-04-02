#include <salgo/sparse-vector.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;






TEST(Sparse_vector, test_push_delete_compact) {

	Sparse_Vector<int> m;
	m.emplace_back(1); //
	m.emplace_back(2);
	m.emplace_back(3); //
	m.emplace_back(4);
	m.emplace_back(5); //

	EXPECT_EQ(5, m.count());

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

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

		v[ii].val = rand();
		v[ii].exists = rand()%2 == 1;
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

		v[ii] = rand();
		exists[ii] = rand()%2 == 1;
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

		v[ii] = rand();
		exists[ii] = rand()%2 == 1;
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

			v[ii].val() = rand();
			if(rand()%2) v[ii].erase();
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

			v[ii].val() = rand();
			if(rand()%2) v[ii].erase();
		}

		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v[ii].exists()) result += v[ii].val();
		}
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << result << ", " << diff.count() << endl;
}










TEST(Performance, sparse_vector) {
	const int N = 10'000'000;

	for(int type=0; type<2; ++type) {

		cout << endl << (type==0 ? "SEQUENTIAL" : "RANDOM ACCESS") << endl;

		run_vector(N, type);
		run_vector_inplace(N,type);
		run_vector_external(N,type);
		run_vector_external_bitset(N,type);
		run_sparse_vector(N,type);
		run_sparse_vector_index(N,type);
	}
}


