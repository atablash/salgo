#include <salgo/named-arguments>

#include <gtest/gtest.h>

using namespace salgo;



NAMED_ARGUMENT(ALPHA)
NAMED_ARGUMENT(BETA)
NAMED_ARGUMENT(GAMMA)
NAMED_ARGUMENT(DELTA)





struct S {
    int x;
    operator int() const { return x; }
    S(int xx) : x(xx) {}
    S(const S&) = delete;
    S(S&&) = delete;
};




TEST(Named_Arguments, simple) {
    auto library_fun = [](int first_arg, auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );

        EXPECT_EQ(first_arg, 69);

        EXPECT_TRUE( args.has(ALPHA) );
        EXPECT_EQ( args(ALPHA), 123 );

        EXPECT_FALSE( args.has(BETA) );

        EXPECT_TRUE( args.has(GAMMA) );
        EXPECT_EQ( args(GAMMA), 234 );
    };

	library_fun(69, GAMMA = 234, ALPHA = S(123));
}

TEST(Named_Arguments, const) {
    auto library_fun = [](auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );
        
        EXPECT_TRUE( args.has(ALPHA) );
        EXPECT_EQ( args(ALPHA), 123 );

        using Type = std::remove_reference_t< decltype(args(ALPHA)) >;
        static_assert( std::is_const_v<Type> );
    };

	library_fun(ALPHA = S(123));
}


TEST(Named_Arguments, pass) {
    auto another_fun = [](auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );

        EXPECT_TRUE( args.has(GAMMA) );
        EXPECT_EQ( args(GAMMA), 99 );

        EXPECT_FALSE( args.has(BETA) );

        EXPECT_TRUE( args.has(ALPHA) );
        EXPECT_EQ( args(ALPHA), 123 );
    };

    auto library_fun = [&another_fun](int first_arg, auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );

        EXPECT_EQ(first_arg, 69);

        EXPECT_TRUE( args.has(ALPHA) );
        EXPECT_EQ( args(ALPHA), 123 );

        EXPECT_FALSE( args.has(BETA) );

        EXPECT_TRUE( args.has(GAMMA) );
        EXPECT_EQ( args(GAMMA), 234 );

        another_fun(
            GAMMA = 99,
            BETA  = args,
            ALPHA = args
        );
    };

	library_fun(69, GAMMA=234, ALPHA=S(123));
}




TEST(Named_Arguments, const_pass) {
    auto another_fun = [](auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );

        EXPECT_TRUE( args.has(ALPHA) );
        EXPECT_EQ( args(ALPHA), 111 );

        EXPECT_TRUE( args.has(BETA) );
        EXPECT_EQ( args(BETA), 222 );

        EXPECT_TRUE( args.has(GAMMA) );
        EXPECT_EQ( args(GAMMA), 333 );

        EXPECT_TRUE( args.has(DELTA) );
        EXPECT_EQ( args(DELTA), 444 );

        using Alpha_Type = std::remove_reference_t< decltype(args(ALPHA)) >;
        static_assert( ! std::is_const_v<Alpha_Type> );

        using Beta_Type  = std::remove_reference_t< decltype(args(BETA)) >;
        static_assert( std::is_const_v<Beta_Type> );

        using Gamma_Type = std::remove_reference_t< decltype(args(GAMMA)) >;
        static_assert( std::is_const_v<Gamma_Type> );

        using Delta_Type = std::remove_reference_t< decltype(args(DELTA)) >;
        static_assert( std::is_const_v<Delta_Type> );
    };

    auto library_fun = [&another_fun](auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );

        another_fun(
            ALPHA << args,
            BETA   = args,
            GAMMA << args,
            DELTA  = args
        );
    };

	library_fun( ALPHA << S(111), BETA << S(222), GAMMA = S(333), DELTA = S(444));
}




TEST(Named_Arguments, forward) {
    auto another_fun = [](auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );

        EXPECT_TRUE( args.has(GAMMA) );
        EXPECT_EQ( args(GAMMA), 99 );

        EXPECT_FALSE( args.has(BETA) );

        EXPECT_TRUE( args.has(ALPHA) );
        EXPECT_EQ( args(ALPHA), 123 );

        return std::is_rvalue_reference_v<decltype(args(ALPHA))>;
    };

    auto library_fun = [&another_fun](int first_arg, auto&&... _args) {
        auto args = Named_Arguments( std::forward<decltype(_args)>(_args)... );

        EXPECT_EQ(first_arg, 69);

        EXPECT_TRUE( args.has(ALPHA) );
        EXPECT_EQ( args(ALPHA), 123 );

        EXPECT_FALSE( args.has(BETA) );

        EXPECT_TRUE( args.has(GAMMA) );
        EXPECT_EQ( args(GAMMA), 234 );

        EXPECT_FALSE( another_fun(GAMMA = 99, ALPHA = args,           BETA = args) );
        EXPECT_TRUE ( another_fun(GAMMA = 99, ALPHA = args.forward(), BETA = args.forward()) );

        // todo: maybe forbid accessing argument after being moved-out?
    };

	library_fun(69, GAMMA=234, ALPHA=S(123));
}

TEST(Named_Arguments, empty) {
    auto library_fun = [](auto&&... _args) {
        auto args = Named_Arguments{ std::forward<decltype(_args)>(_args)... };
        (void)args;
    };

    library_fun();
}
