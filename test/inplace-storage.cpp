#include <salgo/inplace-storage>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;







namespace {
	int g_test = 0;
}







TEST(Inplace_Storage, destructor_not_called) {

	struct S{
		~S() {
			++g_test;
		}
	};

	g_test = 0;
	{
		Inplace_Storage<S> storage;

		static_assert( ! std::is_trivially_copy_constructible_v<decltype(storage)> );
		static_assert( ! std::is_trivially_move_constructible_v<decltype(storage)> );

		#ifndef NDEBUG
		static_assert( ! std::is_trivially_destructible_v<decltype(storage)> );
		#else
		static_assert(   std::is_trivially_destructible_v<decltype(storage)> );
		#endif
	}

	EXPECT_EQ(0, g_test);
}








TEST(Inplace_Storage, destructor_called) {

	struct S{
		~S() {
			++g_test;
		}
	};

	g_test = 0;	
	{
		Inplace_Storage<S> storage;
		Inplace_Storage<S> storage2;

		storage.construct();
		storage2.construct();

		storage2 = storage;

		storage.destruct();
		storage2.destruct();
	}

	EXPECT_EQ(2, g_test);
}




TEST(Inplace_Storage, trivial) {

	Inplace_Storage<int> storage;

	static_assert( std::is_trivially_copy_constructible_v<decltype(storage)> );
	static_assert( std::is_trivially_move_constructible_v<decltype(storage)> );
	static_assert( std::is_trivially_destructible_v<decltype(storage)> );
}






TEST(Inplace_Storage, copy_noop) {
	using VS = Inplace_Storage<int>::TREAT_AS_VOID;

	int magic = rand();

	Inplace_Storage<int>::TREAT_AS_VOID s1;
	s1.construct(magic);

	auto s2 = (VS*)std::calloc( sizeof(VS), 1 );
	new(s2) VS(s1);
	EXPECT_NE(s1, *s2);

	*s2 = s1;
	EXPECT_NE(s1, *s2);

	*s2 = std::move(s1);
	EXPECT_EQ(magic, s1);
	EXPECT_NE(magic, *s2);


	auto s3 = (VS*)std::calloc( sizeof(VS), 1 );
	new(s3) VS( std::move(s1) );
	EXPECT_NE(magic, *s3);

	free(s2);
	free(s3);
}


TEST(Inplace_Storage, copy_as_pod) {
	struct S {
		int val = 0;

		S(const S&) { val = 69; }
	};

	Inplace_Storage<S>::TREAT_AS_POD s;

	auto s2 = s;

	DCHECK_EQ(0, ((S&)s2).val);
}



