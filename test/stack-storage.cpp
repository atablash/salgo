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




