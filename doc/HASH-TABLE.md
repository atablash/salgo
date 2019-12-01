Hash_Table
==========
A replacement for `std::unordered_multiset`, `std::unordered_set`, `std::unordered_multimap`, `std::unordered_map`.

To use as a set (only keys):

```cpp
	Hash_Table<int> s = {11, 22, 33, 44, 55};
	cout << s(34).found() << endl; // false
	s(33).erase(); // now {11, 22, 44, 55}
```

To use as a map (keys + values):

```cpp
	Hash_Table<int, std::string> m = {{12,"twelve"}, {6,"six"}, {3,"three"}, {20,"twenty"}};
	for(auto& e : m) {
		cout << e.key() << " -> " e.val() << endl;
	}
	// prints (in undefined order):
	// 6 -> six
	// 20 -> twenty
	// 3 -> three
	// 12 -> twelve

	m.emplace(2,"two"); // add one more
```




`salgo::Hash`
-------------
Hash_Table uses `salgo::Hash<Key>` by default to get hashes for objects.

The `salgo::Hash<Key>` first checks the `Key` type for member `hash()` function, and uses it if available. Otherwise, it uses `std::hash<Key>`.

If you want to provide a hashing function for your custom type, don't specialize `salgo::Hash`, but either:
* Create a member `hash()` function, or
* Specialize `std::hash<Key>` instead




Type Builder
------------
### ::COUNT

Add a counter so total of number elements can be checked using `count()`.

### ::EXTERNAL

By default, elements are stored inplace. Use `::EXTERNAL` to store handles to elements allocated using Allocator instead.

### ::ALLOCATOR<Alloc>

Allocator used to construct elements. Used only if `::EXTERNAL`.




Safety
------
Scoped `for` (`for(auto& e : c)`):
* Erase current element via iterator: yes
* Erase current element externally: yes
* Erase other element: no




Performance (x86_64)
--------------------
Check Travis builds for full benchmarks using gcc/clang and libstdc++/libc++.

Comparing to `std::unordered_multiset` from `libstdc++`.

|Benchmark         |    Salgo|   libstdc++|
|------------------|--------:|-----------:|
|REHASH (~1M)      |51 ms    |51 ms       |
|INSERT            |138 ns   |215 ns      |
|REVERVED_INSERT   |72 ns    |149 ns      |
|FIND              |51 ns    |83 ns       |
|FIND_ERASE        |64 ns    |134 ns      |
|ITERATE           |7 ns     |30 ns       |




More Examples
-------------
See `test/hash-table.cpp` for more usage examples.



TODO
----
* Implement not-multi version, e.g. by `::UNIQUE` type parameter.
* Expose parameter for bucket INPLACE_BUFFER
* implement initializer lists, actually...


