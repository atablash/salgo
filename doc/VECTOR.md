Vector
======
In a basic form, similar to `std::vector`. Optionally, you can enable some additional functionality:
* Spare vector - some elements can be erased and become non-existing
	* `exists` flags can be stored to make the object aware of which elements are present
	* A *counter* can keep track of the number of non-erased elements
* For optimization, an in-place buffer can be created to keep a number of elements in-place (like `std::array`)




Type Builder
------------
By default, a minimal object is constructed:

	Vector<int> v;

### ::SPARSE

Allows erasing elements in the middle of the vector, and leaving "holes".

Also enables `::CONSTRUCTED_FLAGS`.

> NOTE
>
> If you want just sparse, but not `::CONSTRUCTED_FLAGS`, use `::SPARSE_NO_CONSTRUCTED_FLAGS` instead, but it'll work only for trivially move constructible types (implies trivially destructible as well).


### ::CONSTRUCTED_FLAGS (default on if ::SPARSE)

For sparse vectors, makes the *Vector* aware of which elements are existing.

This allows resizing (move construction on reallocation), iteration and destruction of elements (if not trivially destructible).

	Vector<int> ::SPARSE v = {1, 2, 33, 4, 5};
	for(auto& e : v) if(e > 10) e.erase();
	// now {1, 2, _, 4, 5}

	for(const auto& e : v) cout << e << ', '; // 1, 2, 4, 5, 

> NOTE
>
> By default, it keeps `exists` flags in a `std::bitset`-like structure.
>
> You can also choose to keep the flags in-place with the elements, if it works faster for you (probably won't).
>
> You can choose from these two using `::CONSTRUCTED_FLAGS_BITSET` or `::CONSTRUCTED_FLAGS_INPLACE`. If so, don't include the `::CONSTRUCTED_FLAGS` selector!
>
> `::CONSTRUCTED_FLAGS` maps to `::CONSTRUCTED_FLAGS_BITSET`.


### ::COUNT

Adds a counter, so makes the *Vector* aware of how many existing elements it has (in constant asymptotic time).

	Vector<int> ::SPARSE ::COUNT v = {1, 2, 3, 4, 5};
	v(2).erase();
	cout << "have " << v.count() << " elements" << endl; // have 4 elements

### ::INPLACE_BUFFER<N>

Adds inplace buffer to the object, of capacity N elements. When the Vector has at most N elements, they'll be stored inplace (as with `std::array`).




Construction
------------
	Vector<int> a;    // en empty vector
	Vector<int> b(10);    // a vector of initial size 10, elements default-constructed
	Vector< std::pair<int,double> > c(10,   69, 1.0);    // a vector of initial size 10, all elements {69,1.0}
	Vector<int> d = {1, 2, 3}; // a vector with elements {1,2,3}




Accessing elements
------------------
	Vector<int> ::SPARSE v = {0, 1, 22, 3, 4};
	v[2] = 2;     // operator[] -> by reference
	v(2).erase(); // operator() -> by accessor




Loops and erasing elements
--------------------------
It's safe to use either `Vector::Accessor::erase()` or `Vector::erase(handle)` within loops.

	Vector<int> ::SPARSE v = {1, 2, 33, 4, 5};
	for(auto& e : v) if(e > 10) e.erase(); // safe!




Loops and changing size
-----------------------
Range-based-for caches `end()` iterator, and erasing or pushing back elements changes the end() iterator position. Still, it works fine with Salgo with no performance overhead - check the source code for details.

```cpp
	Vector<int> v = {1, 2, 3};
	for(auto& e : v) {
		if(e < 10) v.emplace_back(e+10);
	}
	// 1, 2, 3, 11, 12, 13
```

> NOTE
>
> One thing to keep in mind is that there's a special End_Iterator type that `end()` returns. It's identical to the normal iterator, except it acts as a "reference" to `end()` when used on right-hand-side of `!=` (and only then).




Accessors and handles invalidation
----------------------------------
Both accessors/iterators and handles never invalidate (except when the Vector object is moved). TODO: maybe allocate

Handles and accessors/iterators to erased elements are still valid.

Pointers to elements can invalidate, as elements can be moved when growing the Vector (TODO: option to reallocation also on shrink). Contrary to accessors and handles, pointers don't invalidate when moving the `Vector` object.




Performance (x86_64)
--------------------

Check Travis builds for full benchmarks using gcc/clang and libstdc++/libc++.

Comparing to `std::vector` from `libstdc++`.

Grow factor is `1.5` for Salgo.

| Benchmark         |     Salgo | libstdc++ |
|-------------------|----------:|----------:|
| PUSH_BACK         |     10 ns |     10 ns |
| RANDOM_ACCESS     |     28 ns |     29 ns |
| SEQUENTIAL_ACCESS |    350 us |    117 us |
| FOREACH_ACCESS    |    351 us |    329 us |



More Examples
-------------
See `test/vector.cpp` and `vector-sparse.cpp` for more usage examples.




TODO
----
* Auto-shrink option

