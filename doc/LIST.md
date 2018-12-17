List
====
Similar to `std::list`.




Performance (x86_64)
--------------------
Check Travis builds for full benchmarks using gcc/clang and libstdc++/libc++.

Comparing `salgo::List<int> ::COUNT` to `std::list<int>` from `libstdc++`, compiled using `clang++-5.0`.

|Benchmark         |    Salgo| Salgo + std::allocator|   libstdc++|
|------------------|--------:|----------------------:|-----------:|
|INSERT_ERASE      |19 ns    |37 ns                  |23 ns       |
|ITERATE           |5 ns     |13 ns                  |12 ns       |
|INSERT            |48 ns    |81 ns                  |95 ns       |
|ERASE             |6 ns     |18 ns                  |13 ns       |


> NOTE
>
> Crude_Allocator is unable to reuse memory - it keeps all the memory allocated, until Crude_Allocator is destructed.
>
> Use it only if you know what you're doing.

