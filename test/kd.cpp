#include <salgo/linalg/kd>

#include <gtest/gtest.h>

#include <Eigen/Dense>

#include <random>
#include <algorithm>

using namespace salgo::linalg;




TEST(Kd, simple) {
	using Vec2 = Eigen::Matrix<double, 2, 1>;

	Kd ::AABB_OF<Vec2> kd;
	kd.insert({1,2});
}



TEST(Kd, simple_key) {
	using Vec2 = Eigen::Matrix<double, 2, 1>;

	Kd ::KEY<Vec2> kd;
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
	Kd ::KEY<Vec4> ::ALIGN<32> kd; // 32-byte alignment for Eigen AlignedBox of Vec4

	insert_grid(kd);

	auto v = kd.find_closest_center({1.49, 3.88, 0.12, 2.7});
	ASSERT_TRUE( v.found() );
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
	Kd ::KEY<Vec4> ::VAL<int> ::ALIGN<32> kd; // 32-byte alignment for Eigen AlignedBox of Vec4

	insert_grid_keyval(kd);

	auto v = kd.find_closest_center({1.499999, 2.9, 3.9, 0.500001});
	ASSERT_TRUE( v.found() );
	EXPECT_EQ( v.key(), Vec4(1,3,4,1)  );
	EXPECT_EQ( v.val(), 1000*1 + 100*3 + 10*4 + 1 );
}

TEST(Kd, find_closest_all_erased) {
	Kd ::KEY<Vec4> ::VAL<int> ::ALIGN<32> ::ERASABLE kd; // 32-byte alignment for Eigen AlignedBox of Vec4

	insert_grid_keyval(kd);

	kd.each_intersecting(Eigen::AlignedBox<double,4>{
		Vec4{-1000, -1000, -1000, -1000},
		Vec4{1000, 1000, 1000, 1000}}, [](auto& node) {
			node.erase();
		});
	
	auto yet_another = kd.insert({1,2,3,4}, 123);
	yet_another.erase();

	auto r = kd.find_closest_center({1,2,3,4});
	EXPECT_TRUE( r.not_found() );

	int num_found = 0;
	kd.each_intersecting(Eigen::AlignedBox<double,4>{
		Vec4{-1000, -1000, -1000, -1000},
		Vec4{1000, 1000, 1000, 1000}}, [&](auto&) {
			++num_found;
		});
	EXPECT_EQ(num_found, 0);
}
