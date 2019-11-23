#include <salgo/hash-table.hpp>
#include <gtest/gtest.h>


using namespace salgo;

TEST(Hash_Table, iterate) {
	Hash_Table<int> ht;
	ht.emplace(3);
	ht.emplace(30);
	ht.emplace(300);
	ht.emplace(3000);
	ht.emplace(30000);
	ht.emplace(300000);

	int sum = 0;
	for(auto& e : ht) {
		sum += e;
	}
	EXPECT_EQ(333333, sum);
}

TEST(Hash_Table, iterate_empty) {
	Hash_Table<int> ht;
	int sum = 0;
	for(auto& e : ht) sum += e;
	EXPECT_EQ(0, sum);
}

TEST(Hash_Table, iterate_empty_2) {
	Hash_Table<int> ht;
	ht.emplace(123);

	if(!ht(12).exists()) ht(123).erase();

	int sum = 0;
	for(auto& e : ht) sum += e;
	EXPECT_EQ(0, sum);
}

TEST(Hash_Table, erase) {
	Hash_Table<int> ht;
	ht.emplace(3);
	ht.emplace(30);
	ht.emplace(300);
	ht.emplace(3000);
	ht.emplace(30000);
	ht.emplace(300000);

	ht(30).erase();
	ht(3000).erase();
	ht(300000).erase();

	int sum = 0;
	for(auto& e : ht) {
		sum += e;
	}

	EXPECT_EQ(30303, sum);
}


TEST(Hash_Table, external_erase_1) {
	Hash_Table<int> ht;
	ht.emplace(1);
	ht.emplace(30);
	ht.emplace(100);
	ht.emplace(3000);
	ht.emplace(10000);
	ht.emplace(100000);

	int sum1 = 0;
	for(auto& e : ht) {
		if(e % 3 == 0) ht( e.handle() ).erase();
		else sum1 += e;
	}

	int sum2 = 0;
	for(auto& e : ht) sum2 += e;

	EXPECT_EQ(110101, sum1);
	EXPECT_EQ(110101, sum2);
}

TEST(Hash_Table, external_erase_2) {
	Hash_Table<int> ht;
	ht.emplace(1);
	ht.emplace(30);
	ht.emplace(100);
	ht.emplace(3000);
	ht.emplace(10000);
	ht.emplace(100000);

	int sum1 = 0;
	for(auto& e : ht) {
		if(e % 3 != 0) ht( e.handle() ).erase();
		else sum1 += e;
	}

	int sum2 = 0;
	for(auto& e : ht) sum2 += e;

	EXPECT_EQ(3030, sum1);
	EXPECT_EQ(3030, sum2);
}


TEST(Hash_Table, initializer_list) {
	Hash_Table<int> ht = {1, 10, 10000, 1000, 100};
	int sum = 0;
	for(auto& e : ht) sum += e;
	EXPECT_EQ(11111, sum);
}


TEST(Hash_Table, key_val) {
	Hash_Table<int, std::string> m = {{12,"twelve"}, {6,"six"}, {3,"three"}, {20,"twenty"}};
	std::map<int,std::string> test;
	for(auto& e : m) {
		test.emplace(std::pair(e.key(), e.val()));
	}
	EXPECT_EQ(4, (int)test.size());
	EXPECT_EQ(test[12], "twelve");
	EXPECT_EQ(test[6],  "six");
	EXPECT_EQ(test[3],  "three");
	EXPECT_EQ(test[20], "twenty");
}


TEST(Hash_Table, copy) {
	Hash_Table<int> _ht = {1, 100, 10000};
	auto ht = _ht;

	{
		int sum = 0;
		for(auto& e : ht) sum += e;
		EXPECT_EQ(10101, sum);
	}

	{
		int sum = 0;
		for(auto& e : _ht) sum += e;
		EXPECT_EQ(10101, sum);
	}
}

TEST(Hash_Table, move) {
	Hash_Table<int> _ht = {1, 100, 10000};
	auto ht = std::move(_ht);

	{
		int sum = 0;
		for(auto& e : ht) sum += e;
		EXPECT_EQ(10101, sum);
	}

	{
		int sum = 0;
		for(auto& e : _ht) sum += e;
		EXPECT_EQ(0, sum);
	}
}


// TODO: add tests for non-movable, non-copyable types
