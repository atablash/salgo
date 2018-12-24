#include <salgo/modulo.hpp>

#include <gtest/gtest.h>


using namespace std;
using namespace salgo;




TEST(Modulo, simple) {
	Modulo ::MOD<11> m;
	m += 5; // 5
	m *= 200;
	m /= 100; // 10
	m -= 7; // 3
	EXPECT_EQ(3, m);
}



TEST(Modulo, contexted) {
	Modulo m;
	auto c = Modulo_Context(11);
	m += 5; // 5
	m *= 200;
	m /= 100; // 10
	m -= 7; // 3
	EXPECT_EQ(3, m);
}

