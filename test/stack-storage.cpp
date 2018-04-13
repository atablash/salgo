#include <salgo/stack-storage.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace salgo;
using namespace std::chrono;







namespace {
	int g_test = 0;
}







TEST(Stack_storage, destructor_not_called) {

	struct S{
		~S() {
			++g_test;
		}
	};

	g_test = 0;
	{
		Stack_Storage<S> storage;

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








TEST(Stack_storage, destructor_called) {

	struct S{
		~S() {
			++g_test;
		}
	};

	g_test = 0;	
	{
		Stack_Storage<S> storage;
		Stack_Storage<S> storage2;

		storage.construct();
		storage2.construct();

		storage2 = storage;

		storage.destruct();
		storage2.destruct();
	}

	EXPECT_EQ(2, g_test);
}




TEST(Stack_storage, trivial) {

	Stack_Storage<int> storage;

	static_assert( std::is_trivially_copy_constructible_v<decltype(storage)> );
	static_assert( std::is_trivially_move_constructible_v<decltype(storage)> );
	static_assert( std::is_trivially_destructible_v<decltype(storage)> );
}






TEST(Stack_storage, copy_noop) {
	using VS = Stack_Storage<int>::TREAT_AS_VOID;

	int magic = rand();

	Stack_Storage<int>::TREAT_AS_VOID s1;
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


TEST(Stack_storage, copy_as_pod) {
	struct S {
		int val = 0;

		S(const S&) { val = 69; }
	};

	Stack_Storage<S>::TREAT_AS_POD s;

	auto s2 = s;

	DCHECK_EQ(0, ((S&)s2).val);
}



