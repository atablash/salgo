#include <salgo/kd>

#include <gtest/gtest.h>

#include <Eigen/Dense>

#include <random>
#include <algorithm>

using namespace salgo;



TEST(Kd, simple) {
	using Vec2 = Eigen::Matrix<double, 2, 1>;

	Kd<Vec2> kd;
	kd.insert({1,2});
}



using Vec4 = Eigen::Matrix<double, 4, 1>;



template<class KD>
void insert_grid(KD& kd) {

	std::vector<Vec4> v;

	for(int x=0; x<5; ++x) {
		for(int y=0; y<5; ++y) {
			for(int z=0; z<5; ++z) {
				for(int w=0; w<5; ++w) {
					v.emplace_back(x, y, z, w);
				}
			}
		}
	}

    std::random_device rd;
    std::mt19937 g(rd());
	std::shuffle(v.begin(), v.end(), g);

	for(auto& p : v) {
		kd.insert(p);
	}
}



TEST(Kd, find_closest) {
	Kd<Vec4> ::ALIGN<32> kd; // 32-byte alignment for Eigen AlignedBox of Vec4

	insert_grid(kd);

	auto v = kd.find_closest_center({1.49, 3.88, 0.12, 2.7});
	ASSERT_TRUE( v.exists() );
	EXPECT_EQ( Vec4(1,4,0,3), v.key() );
}



template<class KD>
void insert_grid_keyval(KD& kd) {
	std::vector< std::pair<Vec4,int> > v;

	for(int x=0; x<5; ++x) {
		for(int y=0; y<5; ++y) {
			for(int z=0; z<5; ++z) {
				for(int w=0; w<5; ++w) {
					v.push_back({
						{1.0*x, 1.0*y, 1.0*z, 1.0*w},
						1000*x + 100*y + 10*z + w
					});
				}
			}
		}
	}

    std::random_device rd;
    std::mt19937 g(rd());
	std::shuffle(v.begin(), v.end(), g);

	for(auto& p : v) {
		kd.emplace(p);
	}
}


TEST(Kd, find_closest_val) {
	Kd<Vec4, int> ::ALIGN<32> kd; // 32-byte alignment for Eigen AlignedBox of Vec4

	insert_grid_keyval(kd);

	auto v = kd.find_closest_center({1.499999, 2.9, 3.9, 0.500001});
	ASSERT_TRUE( v.exists() );
	EXPECT_EQ( v.key(), Vec4(1,3,4,1)  );
	EXPECT_EQ( v.val(), 1000*1 + 100*3 + 10*4 + 1 );
}
