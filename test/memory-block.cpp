#include <salgo/memory-block.hpp>

#include <gtest/gtest.h>

using namespace salgo;







namespace {
	int g_destructors = 0;
	int g_constructors = 0;
}





TEST(Memory_block, stack_optim_inplace_nontrivial) {

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
		Memory_Block<S>::EXISTS_INPLACE::COUNT::INPLACE_BUFFER<2> m(1);
		m(0).construct(1);
		m.resize(2);
		m(1).construct(2);
		m.resize(3);
		m(2).construct(3);

		EXPECT_EQ(1, m(0)());
		EXPECT_EQ(2, m(1)());
		EXPECT_EQ(3, m[2]);

		m(1).destruct();

		EXPECT_TRUE ( m(0).exists() );
		EXPECT_FALSE( m(1).exists() );

		int sum = 0;
		for(auto& e : m) {
			sum += e();
		}
		EXPECT_EQ(4, sum);
	}


	EXPECT_EQ(g_constructors, g_destructors);
}



TEST(Memory_block, destructors_called_exists) {
	struct S {
		S()  { ++g_constructors; }
		S(const S&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Memory_Block<S>::EXISTS::COUNT block(10);
		block.construct_all();

		EXPECT_EQ(10, block.size());
		EXPECT_EQ(10, block.count());
	}

	EXPECT_EQ(g_constructors, g_destructors);
}



TEST(Memory_block, destructors_called_dense) {
	struct S {
		S()  { ++g_constructors; }
		S(S&&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Memory_Block<S>::DENSE block(10);

		EXPECT_EQ(10, block.size());
		EXPECT_EQ(10, block.count());
	}

	EXPECT_EQ(g_constructors, g_destructors);
}


TEST(Memory_block, destructors_called_dense_resized) {
	struct S {
		S()  { ++g_constructors; }
		S(S&&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Memory_Block<S>::DENSE block;
		EXPECT_EQ(0, block.size());
		EXPECT_EQ(0, block.count());
		block.resize(1);

		EXPECT_EQ(1, block.size());
		EXPECT_EQ(1, block.count());
	}

	EXPECT_EQ(g_constructors, g_destructors);
}





TEST(Memory_block, dense_constructor) {
	Memory_Block<int>::DENSE mb(10, 10);

	int sum = 0;
	for(auto& e : mb) sum += e;
	EXPECT_EQ(100, sum);
}






TEST(Memory_block, copy_container_exists) {
	struct S {
		S()  { ++g_constructors; }
		S(const S&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Memory_Block<S>::EXISTS block(10);
		block.construct_all();

		auto block2 = block;
	}

	EXPECT_EQ(g_constructors, g_destructors);
}

TEST(Memory_block, copy_container_dense) {
	struct S {
		S()  { ++g_constructors; }
		S(const S&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Memory_Block<S>::DENSE block(10);
		//block.construct_all();

		auto block2 = block;
		block2 = block;
	}

	EXPECT_EQ(g_constructors, g_destructors);
}





TEST(Memory_block, move_container_exists) {
	struct S {
		S()  { ++g_constructors; }
		S(S&&) { ++g_constructors; }
		~S() { ++g_destructors;  }
	};

	g_destructors = 0;
	g_constructors = 0;

	{
		Memory_Block<S>::EXISTS::INPLACE_BUFFER<2> block(10);
		block.construct_all();

		auto block2 = std::move(block);
		block2 = std::move(block);
	}

	EXPECT_EQ(g_constructors, g_destructors);
}



