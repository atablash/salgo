#include "common.hpp"

#include <salgo/memory-block>

#include <gtest/gtest.h>

using namespace salgo;







TEST(Memory_Block, stack_optim_inplace_nontrivial) {
	using T = Movable;
	T::reset();

	{
		Memory_Block<T> ::CONSTRUCTED_FLAGS_INPLACE ::COUNT ::INPLACE_BUFFER<2> m(1);
		m(0).construct(1);
		m.resize(2);
		m(1).construct(2);
		m.resize(3);
		m(2).construct(3);

		EXPECT_EQ(1, m(0)());
		EXPECT_EQ(2, m(1)());
		EXPECT_EQ(3, m[2]);

		m(1).destruct();

		EXPECT_TRUE ( m(0).constructed() );
		EXPECT_FALSE( m(1).constructed() );

		int sum = 0;
		for(auto& e : m) {
			sum += e();
		}
		EXPECT_EQ(4, sum);
	}


	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}



TEST(Memory_Block, destructors_called_exists) {
	using T = Movable;
	T::reset();

	{
		Memory_Block<T> ::CONSTRUCTED_FLAGS ::COUNT block(10);
		block.construct_all();

		EXPECT_EQ(10, block.domain());
		EXPECT_EQ(10, block.count());
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}



TEST(Memory_Block, destructors_called_dense) {
	using T = Movable;
	T::reset();

	{
		Memory_Block<T> ::DENSE  block(10);

		EXPECT_EQ(10, block.domain());
		EXPECT_EQ(10, block.count());
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}


TEST(Memory_Block, destructors_called_dense_shrunk) {
	using T = Movable;
	T::reset();

	{
		Memory_Block<T> ::DENSE  block;
		EXPECT_EQ(0, block.domain());
		EXPECT_EQ(0, block.count());
		block.resize(1);

		EXPECT_EQ(1, block.domain());
		EXPECT_EQ(1, block.count());
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}



TEST(Memory_Block, destructors_called_dense_grown) {
	using T = Movable;
	T::reset();

	{
		Memory_Block<T> ::DENSE  mb(10);
		EXPECT_EQ(10, mb.count());

		mb.resize(20);
		EXPECT_EQ(20, mb.count());
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}





TEST(Memory_Block, dense_constructor) {
	Memory_Block<int> ::DENSE  mb(10, 10);

	int sum = 0;
	for(auto& e : mb) sum += e;
	EXPECT_EQ(100, sum);
}






TEST(Memory_Block, copy_container_exists) {
	using T = Copyable;
	T::reset();

	{
		Memory_Block<T> ::CONSTRUCTED_FLAGS block(10);
		block.construct_all();

		auto block2 = block;
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}

TEST(Memory_Block, copy_container_dense) {
	using T = Copyable;
	T::reset();

	{
		Memory_Block<T>::DENSE block(10);
		//block.construct_all();

		auto block2 = block;
		block2 = block;
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}





TEST(Memory_Block, move_container_exists) {
	using T = Movable;
	T::reset();

	{
		Memory_Block<T> ::CONSTRUCTED_FLAGS ::INPLACE_BUFFER<2> block(10);
		block.construct_all();

		auto block2 = std::move(block);
		EXPECT_EQ(10, block2.domain());

		block = std::move(block2);
		EXPECT_EQ(10, block.domain());
	}

	EXPECT_EQ(T::constructors(), T::destructors());
	EXPECT_NE(T::constructors(), 0);
}



TEST(Memory_Block, begin_returns_first_existing) {
	Memory_Block<int> ::CONSTRUCTED_FLAGS block(10);

	block(0).construct(12);
	block(0).destruct();
	
	block(3).construct(123);

	EXPECT_TRUE( block.begin()->exists() );
	EXPECT_EQ( 123, block.begin()->value() );
}
