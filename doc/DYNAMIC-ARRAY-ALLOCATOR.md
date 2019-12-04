Array_Allocator
=======================
It's currently the default allocator used by most Salgo containers.

> NOTE
>
> `Array_Allocator` is not persistent. When constructing an element, previous elements could be moved to a new memory location.

`Array_Allocator` searches for holes in a continuous allocated memory block, in circular fashion.

When congestion reaches 0.5, memory block is resized to twice the current size.

Currently, the memory block is never shrunk.




Allocators Performance (x86_64)
-------------------------------
| Benchmark  | Array_Allocator | Random_Allocator | Crude_Allocator | std::allocator |
|------------|-----------------:|-----------------:|----------------:|---------------:|
| SEQUENTIAL |             8 ns |            10 ns |            4 ns |          34 ns |
| QUEUE      |            18 ns |            21 ns |           17 ns |          33 ns |
| RANDOM     |            31 ns |            47 ns |           23 ns |          67 ns |

`std::allocator` simply proxies requests to the system allocator (Ubuntu 16.04).

> NOTE
>
> Crude_Allocator is unable to reuse memory - it keeps all the memory allocated, until Crude_Allocator is destructed.
>
> Use it only if you know what you're doing.

