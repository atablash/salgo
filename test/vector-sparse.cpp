#include <salgo/vector.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;







TEST(Sparse_vector, not_iterable) {

	Vector<int>::SPARSE m;
	m.emplace_back(1);
	m.emplace_back(2);
	m.emplace_back(3);
	m.emplace_back(4);
	m.emplace_back(5);

	EXPECT_EQ(5, m.domain());

	EXPECT_EQ(1, m[0]);
	EXPECT_EQ(2, m(1)());
	EXPECT_EQ(3, m[2]);
	EXPECT_EQ(4, m[3]);
	EXPECT_EQ(5, m[4]);

	m(0).destruct();
	m(1).destruct();
	m(2).destruct();
	m(3).destruct();
	m(4).destruct();
}







TEST(Sparse_vector, not_iterable_nontrivial) {


	struct A {
		int val;
		int* counter;

		A(int v, int* p) : val(v), counter(p) {}
		~A() { ++*counter; }
	};


	Vector< unique_ptr<A> >::SPARSE m(3);
	// m.emplace_back( make_unique<int>(1) ); // should not compile

	EXPECT_EQ(3, m.domain());
	EXPECT_EQ(3, m.capacity());

	int counter = 0;

	m[0] = make_unique<A>(1,&counter);
	m[1] = make_unique<A>(2,&counter);
	m[2] = make_unique<A>(3,&counter);

	EXPECT_EQ(1, m(0)()->val);
	EXPECT_EQ(2, m[1]->val);
	EXPECT_EQ(3, m(2)()->val);

	m(0).destruct();
	m(1).destruct();
	m(2).destruct();

	EXPECT_EQ(3, counter);
}



template<class MB>
static void push_delete_compact_common(MB& m) {
	m.emplace_back(1); //
	m.emplace_back(2);
	m.emplace_back(3); //
	m.emplace_back(4);
	m.emplace_back(5); //

	EXPECT_EQ(5, m.count());

	//EXPECT_EQ(0, m.domain_begin());
	//EXPECT_EQ(5, m.domain_end());

	EXPECT_EQ(5, m.domain());

	EXPECT_EQ(1, m(0)());
	EXPECT_EQ(2, m(1)());
	EXPECT_EQ(3, m(2)());
	EXPECT_EQ(4, m(3)());
	EXPECT_EQ(5, m(4)());

	{
		int sum = 0;
		for(const auto& e : m) sum += e();
		EXPECT_EQ(15, sum);
	}

	m(0).destruct();
	m(2).destruct();
	m(4).destruct();

	EXPECT_TRUE(  m(1).constructed() );
	EXPECT_FALSE( m(2).constructed() );

	{
		int sum = 0;
		for(const auto& e : m) sum += e();
		EXPECT_EQ(6, sum);
	}

	{
		vector<pair<int,int>> remap;
		m.compact([&](int fr, int to){ remap.emplace_back(fr,to); });
		EXPECT_EQ( decltype(remap)({{1,0}, {3,1}}), remap );
	}

	{
		int sum = 0;
		for(auto& e : m) sum += e();
		EXPECT_EQ(6, sum);
	}
}



TEST(Sparse_vector, push_delete_compact_inplace) {

	Vector<int>::SPARSE::CONSTRUCTED_FLAGS_INPLACE::COUNT m;
	push_delete_compact_common(m);
}





TEST(Sparse_vector, push_delete_compact_bitset) {

	Vector<int>::SPARSE::CONSTRUCTED_FLAGS_BITSET::COUNT m;
	push_delete_compact_common(m);
}






namespace {
	int g_destructors = 0;
	int g_constructors = 0;
}

TEST(Sparse_vector, push_delete_compact_inplace_nontrivial) {

	struct S {
		int val;

		S(int v) : val(v) {
			++g_constructors;
		}
		
		operator int() const {return val;}

		~S() {
			++g_destructors;
		}

		S(S&& o) : val(o.val) {
			++g_constructors;
		}
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Vector<S>::SPARSE::CONSTRUCTED_FLAGS_INPLACE::COUNT m;
		push_delete_compact_common(m);
	}

	EXPECT_EQ(g_constructors, g_destructors);
}






TEST(Sparse_vector, copy_container_exists) {
	struct S {
		S()  { ++g_constructors; }
		S(const S&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Vector<S>::SPARSE::CONSTRUCTED_FLAGS block;
		block.emplace_back();
		block.emplace_back();
		block.emplace_back();

		auto block2 = block;
		block2 = block;
	}

	EXPECT_EQ(g_constructors, g_destructors);
}


TEST(Sparse_vector, move_container_exists) {
	struct S {
		S()  { ++g_constructors; }
		S(S&&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Vector<S>::SPARSE::CONSTRUCTED_FLAGS::INPLACE_BUFFER<2> block(10);
		block.emplace_back();
		block.emplace_back();
		block.emplace_back();

		auto block2 = std::move(block);
		EXPECT_EQ(13, block2.domain());

		block = std::move(block2);
		EXPECT_EQ(13, block.domain());
	}

	EXPECT_EQ(g_constructors, g_destructors);
}


















/*

static void run_vector(int N, int type) {
	

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



static void run_vector_inplace(int N, int type) {
	

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


static void run_vector_external(int N, int type) {
	
	
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


static void run_vector_external_bitset(int N, int type) {
	

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










template<class VEC>
static void run_sparse_vector_common(VEC& v, int N, int type) {
	long long result = 0;

	if(type == 0) {
		// sequential
		for(auto& e : v) {
			e() = rand();
			if(rand()%2) e.destruct();
		}

		for(const auto& e : v) {
			result += e();
		}
	}
	else {
		// random access
		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v(ii).exists()) {
				v(ii)() = rand();
				if(rand()%2) v(ii).destruct();
			}
		}

		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v(ii).exists()) result += v(ii)();
		}
	}

}







static void run_sparse_vector(int N, int type) {
	

	cout << "Sparse_Vector:\t";
	srand(69);
	auto t0 = steady_clock::now();

	Vector<double>::SPARSE::CONSTRUCTED_FLAGS_BITSET v(N);
	run_sparse_vector_common(v, N, type);

	duration<double> diff = steady_clock::now() - t0;
	cout << diff.count() << endl;
}





static void run_sparse_vector_inplace(int N, int type) {
	

	cout << "Sparse_Vector inplace:\t";
	srand(69);
	auto t0 = steady_clock::now();

	Vector<double>::SPARSE::CONSTRUCTED_FLAGS_INPLACE v(N);
	run_sparse_vector_common(v, N, type);
	
	duration<double> diff = steady_clock::now() - t0;
	cout << diff.count() << endl;
}








static void run_sparse_vector_index(int N, int type) {
	

	cout << "Sparse_Vector index:\t";
	srand(69);
	auto t0 = steady_clock::now();

	Vector<double>::SPARSE::CONSTRUCTED_FLAGS_BITSET v(N);
	long long result = 0;

	if(type == 0) {
		// sequential
		for(int i=0; i<N; ++i) {
			v(i)() = rand();
			if(rand()%2) v(i).destruct();
		}

		for(int i=0; i<N; ++i) {
			if(v(i).exists()) result += v(i)();
		}
	}
	else {
		// random access
		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v(ii).exists()) {
				v(ii)() = rand();
				if(rand()%2) v(ii).destruct();
			}
		}

		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v(ii).exists()) result += v(ii)();
		}
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << diff.count() << endl;
}




static void run_sparse_vector_noacc(int N, int type) {
	

	cout << "Sparse_Vector index, noacc:\t";
	srand(69);
	auto t0 = steady_clock::now();

	Vector<double>::SPARSE::CONSTRUCTED_FLAGS_BITSET v(N);
	long long result = 0;

	if(type == 0) {
		// sequential
		for(int i=0; i<N; ++i) {
			v[i] = rand();
			if(rand()%2) v.destruct(i);
		}

		for(int i=0; i<N; ++i) {
			if(v.exists(i)) result += v[i];
		}
	}
	else {
		// random access
		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v.exists(ii)) {
				v[ii] = rand();
				if(rand()%2) v.destruct(ii);
			}
		}

		for(int i=0; i<N; ++i) {
			int ii = rand() % N;

			if(v.exists(ii)) result += v[ii];
		}
	}

	duration<double> diff = steady_clock::now() - t0;
	cout << diff.count() << endl;
}



*/

