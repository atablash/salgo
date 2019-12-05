#include <salgo/dynamic-array>
#include <salgo/list>

#include <gtest/gtest.h>

using namespace salgo;




TEST(Map, simple) {
    Dynamic_Array<int> arr = {1, 2, 3, 4, 5};

    auto squared = arr.map(
        [](auto& el) {
            return el*el;
        }
    );

    auto sum = 0;
    for(auto& e : squared) sum += e;

    EXPECT_EQ(sum, 1*1 + 2*2 + 3*3 + 4*4 + 5*5);
}



TEST(Map, chain) {
    Dynamic_Array<int> arr = {1, 2, 3, 4, 5};

    auto chained = arr.map(
        [](auto& el) {
            return el*el;
        }
    ).map(
        [](auto&& el) {
            return el == 25 ? 1000 : el;
        }
    );

    auto sum = 0;
    for(auto& e : chained) sum += e;

    EXPECT_EQ(sum, 1*1 + 2*2 + 3*3 + 4*4 + 1000);
}



TEST(Map, modify) {
    struct S {
        int number = 0;

        S(int x) : number(x) {}
    };

    Dynamic_Array<S> arr = {1, 2, 3, 4, 5};

    auto numbers = arr.map(
        [](auto& el) -> auto& {
            return el.number;
        }
    );

    for(auto& e : numbers) e *= e;

    auto sum = 0;
    for(auto& e : numbers) sum += e;

    EXPECT_EQ(sum, 1*1 + 2*2 + 3*3 + 4*4 + 5*5);
}



TEST(Map, list_erase) {
    List<int> arr = {1, 2, 3, 4, 5};

    auto squared = arr.map(
        [](auto& el) {
            return el*el;
        }
    );

    for(auto& e : squared) if(e == 9) e.erase();

    auto sum = 0;
    for(auto& e : squared) sum += e;

    EXPECT_EQ(sum, 1*1 + 2*2 + 4*4 + 5*5);
}
