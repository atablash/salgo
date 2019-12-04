#include <salgo/chunked-array>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;



TEST(Chunked_Array, simple) {
	Chunked_Array<int> v;
	v.emplace_back(123);
	EXPECT_EQ(123, v[0]);
}



TEST(Chunked_Array, pop_to_empty) {
	Chunked_Array<int> v;
	for(int i=0; i<10; ++i) v.emplace_back( i );
	for(int i=0; i<10; ++i) EXPECT_EQ(i, v[i]);

	for(int i=0; i<10; ++i) v.pop_back();
	EXPECT_EQ(0, v.size());
}

TEST(Chunked_Array, resize_empty) {
	Chunked_Array<int> v;
	v.resize(10);
	EXPECT_EQ(10, v.size());

	for(int i=0; i<10; ++i) v.pop_back();
	EXPECT_EQ(0, v.size());
}

TEST(Chunked_Array, clear) {
	Chunked_Array<int> v;
	v.resize(10);
	EXPECT_EQ(10, v.size());

	v.clear();
	EXPECT_EQ(0, v.size());
}





namespace {
	int g_constructors = 0;
	int g_destructors = 0;
}

struct Simple {
	Simple() { ++g_constructors; }
	Simple(const Simple&) = delete;
	Simple(Simple&&) = delete;

	Simple& operator=(const Simple&) = delete;
	Simple& operator=(Simple&&) = delete;

	~Simple() { ++g_destructors; }
};



TEST(Chunked_Array, non_movable_type_compiles) {
	Chunked_Array<Simple> v;
	v.emplace_back();
	v.pop_back();
}


TEST(Chunked_Array, destructors_called) {
	g_constructors = 0;
	g_destructors = 0;

	{
		Chunked_Array<Simple> v;
		for(int i=0; i<10; ++i) v.emplace_back();
		// have 10 elements
		v.resize(15);
		for(int i=0; i<8; ++i) v.pop_back();
		// have 7 elements
		v.resize(3);
	}

	EXPECT_EQ(g_constructors, g_destructors);
	EXPECT_NE(g_constructors, 0);
}


TEST(Chunked_Array, destructors_called_sparse) {
	g_destructors = 0;
	g_constructors = 0;

	{
		Chunked_Array<Simple> ::SPARSE v;
		for(int i=0; i<6; ++i) v.emplace_back();
		v.resize(10);
		// have 10 elements
		v(0).erase();
		v(3).erase();
		v(4).erase();
		v(5).erase();
		v(9).erase();
		// have 5 elements, domain==10
		v.resize(7);
		// have 3 elements
		for(int i=0; i<2; ++i) v.pop_back(); // leave 1 element
	}

	EXPECT_EQ(g_constructors, g_destructors);
	EXPECT_NE(g_constructors, 0);
}



TEST(Handles, print_pair_handle) {
	Chunked_Array<int> v;
	v.emplace_back();

	std::ostringstream os, os_want;
	os << v(0);

	os_want << "{0,0} @" << &v;

	EXPECT_EQ(os.str(), os_want.str());
}


