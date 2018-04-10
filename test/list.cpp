#include "clear-cache.hpp"

#include <salgo/list.hpp>
#include <salgo/salgo-from-std-allocator.hpp>

#include <gtest/gtest.h>

#include <list>
#include <chrono>

using namespace salgo;
using namespace std;
using namespace std::chrono;



DECLARE_int32(perf);


TEST(List, simple) {

	List<int> m;
	m.emplace_back(3);
	m.emplace_back(4);
	m.emplace_back(5);
	m.emplace_front(2);
	m.emplace_front(1);

	vector<int> vals;

	for(auto e : m) vals.push_back( e.val() );

	EXPECT_EQ(1, vals[0]);
	EXPECT_EQ(2, vals[1]);
	EXPECT_EQ(3, vals[2]);
	EXPECT_EQ(4, vals[3]);
	EXPECT_EQ(5, vals[4]);
}


TEST(List, count) {

	List<int> :: COUNTABLE m;
	m.emplace_back(3);
	m.emplace_back(4);
	m.emplace_back(5);
	m.emplace_front(2);
	m.emplace_front(1);

	EXPECT_EQ(5, m.count());

	vector<int> vals;
	for(auto e : m) vals.push_back( e.val() );

	EXPECT_EQ(1, vals[0]);
	EXPECT_EQ(2, vals[1]);
	EXPECT_EQ(3, vals[2]);
	EXPECT_EQ(4, vals[3]);
	EXPECT_EQ(5, vals[4]);
}

static int g_constructors = 0;
static int g_destructors = 0;

TEST(List, count_nontrivial) {

	struct S {
		int val;

		S(int i) : val(i) { ++g_constructors; }
		~S() { ++g_destructors; }

		operator int() const { return val; }
	};

	List<S> :: COUNTABLE m;
	m.emplace_back(3);
	m.emplace_back(4);
	m.emplace_back(5);
	m.emplace_front(2);
	m.emplace_front(1);

	EXPECT_EQ(5, m.count());

	vector<int> vals;
	for(auto e : m) vals.push_back( e.val() );

	EXPECT_EQ(1, vals[0]);
	EXPECT_EQ(2, vals[1]);
	EXPECT_EQ(3, vals[2]);
	EXPECT_EQ(4, vals[3]);
	EXPECT_EQ(5, vals[4]);
}








auto run_std_list(const int N) {
	clear_cache();

	srand(69);
	auto t0 = steady_clock::now();

	std::list<int> li;

	for(int i=0; i<N/2; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	int ith = 0;
	for(auto it = li.begin(); it != li.end(); ) {
		auto ne = std::next(it);

		if(ith%2) {
			li.erase(it);
		}

		it = ne;
		++ith;
	}

	unsigned long long result = 0;

	for(auto& e : li) {
		result *= 2;
		result += e;
	}

	duration<double> diff = steady_clock::now() - t0;
	return diff.count();
}




auto run_salgo_list(const int N) {
	clear_cache();

	srand(69);
	auto t0 = steady_clock::now();

	salgo::List<int> li;

	for(int i=0; i<N/2; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	int ith = 0;
	for(auto e : li) {

		if(ith%2) e.erase();

		++ith;
	}

	unsigned long long result = 0;

	for(auto e : li) {
		result *= 2;
		result += e.val();
	}

	duration<double> diff = steady_clock::now() - t0;
	return diff.count();
}




auto run_salgo_list_noacc(const int N) {
	clear_cache();

	srand(69);
	auto t0 = steady_clock::now();

	salgo::List<int> li;

	for(int i=0; i<N/2; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	int ith = 0;
	for(auto it = li.begin(); it != li.end(); ) {
		auto ne = std::next(it);

		if(ith%2) {
			(*it).erase();
		}

		it = ne;
		++ith;
	}

	unsigned long long result = 0;

	for(auto e : li) {
		result *= 2;
		result += e.val();
	}

	duration<double> diff = steady_clock::now() - t0;
	return diff.count();
}






auto run_salgo_list_countable(const int N) {
	clear_cache();

	srand(69);
	auto t0 = steady_clock::now();

	salgo::List<int>::COUNTABLE li;

	for(int i=0; i<N/2; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	int ith = 0;
	for(auto e : li) {

		if(ith%2) e.erase();

		++ith;
	}

	unsigned long long result = 0;

	for(auto e : li) {
		result *= 2;
		result += e.val();
	}

	duration<double> diff = steady_clock::now() - t0;
	return diff.count();
}





auto run_salgo_list_countable_malloc(const int N) {
	clear_cache();

	srand(69);
	auto t0 = steady_clock::now();

	using Alloc = Salgo_From_Std_Allocator< std::allocator<int> >;

	salgo::List<int>::COUNTABLE::ALLOCATOR<Alloc> li;

	for(int i=0; i<N/2; ++i) {
		li.emplace_back( rand() );
		li.emplace_front( rand() );
	}

	int ith = 0;
	for(auto e : li) {

		if(ith%2) e.erase();

		++ith;
	}

	unsigned long long result = 0;

	for(auto e : li) {
		result *= 2;
		result += e.val();
	}

	duration<double> diff = steady_clock::now() - t0;
	return diff.count();
}














TEST(List, perf) {
	const int N = FLAGS_perf;
	if(N == 0) return;

	const int n = N/5;

	double t0 = run_std_list(n);
	double t1 = run_salgo_list(n);
	double t2 = run_salgo_list_noacc(n);
	double t3 = run_salgo_list_countable(n);
	double t4 = run_salgo_list_countable_malloc(n);

	cout.precision(3);
	cout << "std::list\t\t" << t0 << endl;
	cout << "salgo::List\t\t" << t1 << endl;
	cout << "salgo::List noacc\t" << t2 << endl;
	cout << "salgo::List countable\t" << t3 << endl;
	cout << "salgo::List countable std::allocator\t" << t4 << endl;
}