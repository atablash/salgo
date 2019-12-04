#pragma once


//
// macro overloading
// https://stackoverflow.com/a/27051616/1123898
//
#define SALGO_VARGS_(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N 
#define SALGO_VARGS(...) SALGO_VARGS_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define SALGO_CONCAT_(a, b) a##b
#define SALGO_CONCAT(a, b) SALGO_CONCAT_(a, b)




