Memory_Block
============
It's the type used for underlying `Vector` implementation. It has the same functionality, except:

* It's by default SPARSE (can be made DENSE using `::DENSE` type builder selector)
* Its elements are first deleted and you have to construct them.
* It doesn't allow automatic resizing on push_back - keeping to vector terminology, it only has capacity, and no size.

The type builder exposes following parameters: `::DENSE`, `::STACK_BUFFER<N>`, `::CONSTRUCTED_FLAGS`, `::CONSTRUCTED_FLAGS_INPLACE`, `::CONSTRUCTED_FLAGS_BITSET`, `::COUNT` (see the `Vector` for some explaination).



See Also
--------
* [Vector](doc/VECTOR.md) - a replacement for `std::vector`
