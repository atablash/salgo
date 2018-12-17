Salgo_From_Std_Allocator
========================
A Salgo allocator that Wraps an allocator concept of the standard library. It uses regular pointers as handles.

To use the default `new` and `delete` (i.e. system allocator directly), use:

```cpp
	template<class T>
	using Alloc = Salgo_From_Std_Allocator< std::allocator<T> >;

	// e.g. List:
	List<int> ::ALLOCATOR<Alloc<T>> my_list;
```

> NOTE
>
> `Salgo_From_Std_Allocator` doesn't use provided allocator directly, but using `std::allocator_traits` instead.




See Also
--------
* [Vector_Allocator](VECTOR-ALLOCATOR.md) - the default Salgo allocator
