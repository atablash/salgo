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
		Memory_Block<S>::EXISTS_INPLACE::COUNT::STACK_BUFFER<2> m(1);
		m(0).construct(1);
		m.resize(2);
		m(1).construct(2);
		m.resize(3);
		m(2).construct(3);

		EXPECT_EQ(1, m(0).val());
		EXPECT_EQ(2, m(1).val());
		EXPECT_EQ(3, m(2).val());

		m(1).destruct();

		EXPECT_TRUE ( m(0).exists() );
		EXPECT_FALSE( m(1).exists() );

		int sum = 0;
		for(auto e : m) {
			sum += e.val();
		}
		EXPECT_EQ(4, sum);
	}


	EXPECT_EQ(g_constructors, g_destructors);
}


