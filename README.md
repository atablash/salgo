[![Build Status](https://travis-ci.org/atablash/salgo.svg?branch=master)](https://travis-ci.org/atablash/salgo)

Salgo
=====
Salgo is an experimental C++17 algorithm template library.

* Very easy to use - modern C++17 interface
	* Concept of Accessors
	* Iterators don't invalidate where you don't want (iterate-and-erase foreach loops, etc.)
* Highly customizable
	* Type builders allow easy compile-time customization of data structures and algorithms
* Performance is critical
	* Lots of optimizations available via type builders
	* Included benchmarks comparing Salgo to libstdc++ and libc++, using GCC and Clang (see **Travis** builds)

It's an experimental project with no backward compatibility, or even versioning.



Library Design
=================

The library is contained in `salgo` namespace, so either type `salgo::` explicitly every time (especially in header files or in case of name conflicts), or just use:

```cpp
	using namespace salgo;
```



Accessors
---------

Accessors, or views (or proxy objects), are objects that provide a simple interface for accessing underlying raw structures.

> **DESIGN NOTE**
>
> This concept is somewhat present in the standard C++ library too, for pseudo-reference objects returned by `std::bitset<N>::operator[]` and `std::vector<bool>::operator[]`.

```cpp
	Vector<int> ::SPARSE v = {1, 100, 2, 200, 3, 300};

	for(auto& e : v) if(e >= 100) e.erase();

	int sum = 0;
	for(auto& e : v) sum += e;
	cout << "sum: " << sum << endl;	// 1 + 2 + 3 = 6
```

> **NOTE**
>
> The type builder parameter `::SPARSE` lets you erase elements.

> **NOTE**
>
> You can't iterate by value: ~~`for(auto e : v)`~~.
>
> However, you can use `for(const auto& e : dm)` normally, to mark *const*-ness of the accessed element.


As you can see in the above example, accessors are implicitly convertible to the raw type reference (`int&` in this case).

If you need to make this explicit, use `operator()`, i.e. call `e()` instead of `e`:

```cpp
	struct S {
		int foo;
	};

	Vector<S> v = {S{6}, S{24}, S{35}, S{46}, S{57}};

	for(const auto& e : v) cout << e().foo << endl;
```

Salgo containers generally return raw element for `operator[]`, and accessor for `operator()`:

```cpp
	Vector<int> ::SPARSE v = {0, 11, 22, 69, 44, 55};
	v[3] = 33;      // operator[] returns raw element
	v(3).erase();   // operator() returns accessor
```

Accessors forward most operators to the underlying object:

```cpp
	Vector<int> v = {10, 20, 30};
	for(auto& e : v) ++e;
	// now v == {11, 21, 31}
```

Accessors are also implicitly convertible to Handles - see below.



Handles
-------
If you want to store a reference to an element, use a **handle**.

	Vector<int> v = {11, 22, 33, 44, 55};
	auto h = v(2).handle(); // get a handle

	// ...

	// get an accessor back from the handle:
	v(h).erase(); // and e.g. erase the element

As you can see, a handle doesn't store any context, and you need the owning object (the *Vector* in this case) to get back an accessor.

Handles are meant for storage. Don't try storing accessors, they're usually big objects and they're meant to be optimized out.

> NOTE
>
> In this case, the handle type is defined as `Vector<int>::Handle`.
>
> Often there's also a `Small_Handle` available, which is a compressed version, meant for storage.
>
> For node-based structures like `salgo::List`, a `Small_Handle` is currently 4 bytes, and `Handle` is 8 bytes.
>
> Use `Small_Handle` only for storage. It has some overhead to decode it back.
>
> There are conversions defined both ways (currently implicit).




Iterators
---------

Salgo accessors and iterators are essentially the same thing, with a different interface. They're convertible both ways.

Getting iterator interface from accessor:

```cpp
	Vector<int> v = {1, 1, 2, 100, 100, 100, 1};
	int sum = 0;
	for(auto& e : v) {
		sum += e;
		if(e == 2) e.iterator() += 3; // skip next 3 elements
	}
	// sum is 1 + 1 + 2 + 1 (skipped 3x 100)
```

Getting accessor interface from iterator:

```cpp
	List<int> v = {1, 2, 3, 4, 5};
	for(auto it = v.begin(); it != v.end(); ++it) {
		if(it.accessor() == 3) it.accessor().erase();
	}
	// now v == {1, 2, 4, 5}
```

The same thing can be accomplished using `operator*` and `operator->`:

```cpp
	List<int> v = {1, 2, 3, 4, 5};
	for(auto it = v.begin(); it != v.end(); ++it) {
		if(*it == 3) it->erase();
	}
	// now v == {1, 2, 4, 5}
```

Of course, in this case it's easier to use the *scoped for*:

```cpp
	List<int> v = {1, 2, 3, 4, 5};
	for(auto& e : v) {
		if(e == 3) e.erase();
	}
	// now v == {1, 2, 4, 5}
```

> NOTE
>
> As you can see in the above example with `List`, it's safe to erase elements inside the loop in most Salgo containers.

Just like accessor, for simplicity, iterator is implicitly convertible to both underlying object reference and *Handle*. To get this explicit, use `operator()` and `handle()` (again, same as with accessor).


Allocators
----------
Salgo uses some custom allocators by default, designed for fixed-size objects.

They are different from `std` Allocators:
* They're only for fixed size objects, no arrays.
* They return handles instead of pointers.
* To retrieve an object by handle, allocator object is required.
* They don't separate allocation and destruction.
* Allocators are allowed to move their elements to different memory locations (but handles don't change). Such allocators are called **non-persistent**.
* Allocators generally auto-destruct their objects when they are destructed. One exception is `salgo::Salgo_From_Std_Allocator` wrapper that relies on the underlying implementation.

> NOTE
>
> It wasn't possible to use standard Allocator concept, because in Salgo handles alone can't be used to retrieve objects - the allocator object is needed for this.


A Story Behind Accessors
------------------------

Let's take a look at **pointers** in *C*.

_Pointing_ object's *const*-ness is different than *pointed* object's *const*-ness:
* `const int*` (or `int const*`) is a pointer to `const int`
* `int* const` is a *const* pointer to `int`

The latter (a *const* pointer) comes in a different semantic form in `C++`: a **reference**. This allows using a declared *reference*'s name as an alias for the pointed object.
 * Note that a *reference* is not only an alias, but also has an associated storage, just like a *pointer* (unless optimized-out by compiler of course). However, contrary to pointers, this storage is not exposed to programmer, and as a consequence it's always *const* conceptually.

Just like *references*, we want to use *accessors* as aliases for underlying objects. So *accessors* pretend to *be* these underlying objects. However, unlike *references*:
 * *accessors* expose some **interface** to these underlying objects
 * *accessors* are self-contained: they store a **context** to keep track of the related/parent objects

However, C++ treats accessors more like *pointers* than *references*. As a resulult, it takes some trickery to make them work a bit more like *references*.

There's another analogy: just like *STL iterator* is an abstraction over a *pointer*, the *Salgo accessor* is an abstraction over a *reference*. However, again, unlike *STL iterator*:
 * *accessor* expose some **interface** for accessing the underlying *pointed* object
 * *accessor* is self-contained: it stores a **context** to keep track of the related/parent objects


### Accessor *const*-ness

Since *pointed object*'s' *const*-ness is something different than *accessor*'s' *const*-ness, all accessor types come in two flavors:
* `Some_Accessor<MUTAB>`
* `Some_Accessor<CONST>`

The first one allows modification of the *pointed object*, while the latter does not.

Because accessors pretend to be *references*, they're conceptually immutable.

However, suppose you'd like to implement a function that takes a *const* graph vertex *accessor*, to forbid modifying or erasing it:

```cpp
	template<class VERTEX>
	void compute_neighbors_sum(const VERTEX& vertex) {
		// ...
	}
```

You might think this won't work the same way as it works with *references*, because it's not really a reference to a vertex, but rather a reference to an *accessor* (so, a bit like a reference to a reference).

However, Salgo passes *const*-ness of an accessor to the underlying vertex.

Generally, the *pointed object* is considered *const*, if at least one of the following conditions is true:
* The *accessor* object is *const*
* It's a `CONST`-flavor accessor



Type Builders
-------------
Instead of providing positional template arguments directly, Salgo uses a concept of Type Builders.

For example, to define a `Vector<int>` that is *sparse* and allows *counting* of its sparse elements, you would declare it as:

	Vector<int> ::SPARSE ::COUNT  v;

So, type builder selectors can be chained.

Some type builder selectors are parametrized:

	Vector<int>
		::SPARSE
		::COUNT
		::ALLOCATOR< Salgo_From_Std_Allocator< std::allocator<int> > >  v;

Of course, you can declare a new type, and parametrize it more later:

	using A = Vector<int> ::SPARSE;
	using B = A ::COUNT; // add a counter

	// or maybe we want doubles now?
	using C = B ::VAL<double>	// still SPARSE and COUNTable



Reference
=========
Apart from the following sections, you can also use the code in `test` directory as a reference.




Vector
------
In a basic form, similar to `std::vector`.

Optionally, you can enable some additional functionality:
* Spare vector - some elements can be erased and become non-existing
	* `exists` flags can be stored to make the object aware of which elements are present
	* A *counter* can keep track of the number of non-erased elements
* For optimization, an in-place buffer can be created to keep a number of elements in-place (like `std::array`)

### Type Builder

By default, a minimal object is constructed:

	Vector<int> v;

#### ::SPARSE

Allows erasing elements in the middle of the vector, and leaving "holes".

Also enables `::CONSTRUCTED_FLAGS`.

> NOTE
>
> If you want just sparse, but not `::CONSTRUCTED_FLAGS`, use `::SPARSE_NO_CONSTRUCTED_FLAGS` instead, but it'll work only for trivially move constructible types (implies trivially destructible as well).


#### ::CONSTRUCTED_FLAGS (default on if ::SPARSE)

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


#### ::COUNT

Adds a counter, so makes the *Vector* aware of how many existing elements it has (in constant asymptotic time).

	Vector<int> ::SPARSE ::COUNT v = {1, 2, 3, 4, 5};
	v(2).erase();
	cout << "have " << v.count() << " elements" << endl; // have 4 elements

#### ::INPLACE_BUFFER<N>

Adds inplace buffer to the object, of capacity N elements. When the Vector has at most N elements, they'll be stored inplace (as with `std::array`).


### Construction

	Vector<int> a;    // en empty vector
	Vector<int> b(10);    // a vector of initial size 10, elements default-constructed
	Vector< std::pair<int,double> > c(10,   69, 1.0);    // a vector of initial size 10, all elements {69,1.0}
	Vector<int> d = {1, 2, 3}; // a vector with elements {1,2,3}


### Accessing elements

	Vector<int> ::SPARSE v = {0, 1, 22, 3, 4};
	v[2] = 2;     // operator[] -> by reference
	v(2).erase(); // operator() -> by accessor



### Loops and erasing elements

It's safe to use either `Vector::Accessor::erase()` or `Vector::erase(handle)` within loops.

	Vector<int> ::SPARSE v = {1, 2, 33, 4, 5};
	for(auto& e : v) if(e > 10) e.erase(); // safe!

### Loops and changing size

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


### Accessors and handles invalidation

Both accessors/iterators and handles never invalidate (except when the Vector object is moved). TODO: maybe allocate

Handles and accessors/iterators to erased elements are still valid.

Pointers to elements can invalidate, as elements can be moved when growing the Vector (TODO: option to reallocation also on shrink). Contrary to accessors and handles, pointers don't invalidate when moving the `Vector` object.

### Performance (x86_64)

Check Travis builds for full benchmarks using gcc/clang and libstdc++/libc++.

Comparing to `std::vector` from `libstdc++`.

Grow factor is `1.5` for Salgo.

| Benchmark         |     Salgo | libstdc++ |
|-------------------|----------:|----------:|
| PUSH_BACK         |     10 ns |     10 ns |
| RANDOM_ACCESS     |     28 ns |     29 ns |
| SEQUENTIAL_ACCESS |    350 us |    117 us |
| FOREACH_ACCESS    |    351 us |    329 us |


### TODO
* Auto-shrink option


Memory_Block
------------
It's the type used for underlying `Vector` implementation. It has the same functionality, except:

* It's by default SPARSE (can be made DENSE using `::DENSE` type builder selector)
* Its elements are first deleted and you have to construct them.
* It doesn't allow automatic resizing on push_back - keeping to vector terminology, it only has capacity, and no size.

The type builder exposes following parameters: `::DENSE`, `::STACK_BUFFER<N>`, `::CONSTRUCTED_FLAGS`, `::CONSTRUCTED_FLAGS_INPLACE`, `::CONSTRUCTED_FLAGS_BITSET`, `::COUNT` (see the `Vector` for some explaination).


Chunked_Vector
--------------
It's similar to `Vector`, but doesn't reallocate. Instead, it creates new memory blocks for the new elements, so it's not contiguous.

You can use it when you have non-movable elements (same as with `std::deque`).

It's used internally in `Random_Allocator`.

### Loops and erasing elements

Safe through both the Accessor object or Chunked_Vector object, same as with `salgo::Vector`.


### Accessors and handles invalidation

Similarly to `salgo::Vector`, accessors/iterators and handles never invalidate, except when the `Chunked_Vector` object is moved.

Pointers to elements never invalidate, because objects are never moved.



Vector_Allocator
----------------
It's currently the default allocator used by most Salgo containers.

> NOTE
>
> `Vector_Allocator` is not persistent. When constructing an element, previous elements could be moved to a new memory location.

`Vector_Allocator` searches for holes in a continuous allocated memory block, in circular fashion.

When congestion reaches 0.5, memory block is resized to twice the current size.

Currently, the memory block is never shrunk.


Random_Allocator
----------------
Built on top of Chunked_Vector. Searches for memory holes by randomly drawing indices.

### TODO
* Allocate in circular fashion instead of drawing random numbers, and rename to Chunked_Allocator.


Crude_Allocator
---------------
Fast persistent allocator that doesn't reuse memory. Generally rather for testing/benchmarking purposes.

### TODO
* Implement using `Chunked_Vector` instead of custom memory blocks implementation. And compare performance of course.


Salgo_From_Std_Allocator
------------------------
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


Allocators Performance (x86_64)
-------------------------------
| Benchmark  | Vector_Allocator | Random_Allocator | Crude_Allocator | std::allocator |
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



Unordered_Vector
----------------
It's a small wrapper around `Vector`. When erasing an element, the last element in the vector is moved to fill the hole.

### Loops and erasing elements

Erasing during backward iteration is always safe.

Erasing during forward iteration is safe only through iterating Accessor, but not though the `Unordered_Vector` object, i.e.:

```cpp
	Unordered_Vector v = {1, 2, 3, 4, 5};
	for(auto& e : v) {
		if(e == 3) {
			e.erase(); // good
			// v.erase(e)   // bad! iteration will skip one element (`5` moved to `3`)
			// v(e).erase() // bad! iteration will skip one element (`5` moved to `3`)
		}
	}
```

Range-based-for caches `end()` iterator, and erasing or pushing back elements changes the end() iterator position. Even though it works fine with Salgo with no performance overhead - check the source code for details.

> NOTE
>
> One thing to keep in mind is that there's a special End_Iterator type that `end()` returns. It's identical to the normal iterator, except it acts as a "reference" to `end()` when used on right-hand-side of `!=` (and only then).


### Performance

Even though forward iteration requires a check if element was just deleted or not (as opposed to backward iteration), there's no overhead as measured in benchmarks, and both forward and backward iteration have the same performance. Backward iteration through salgo::Reversed adapter also has no overhead.


Hash_Table
----------
A replacement for `std::unordered_multiset`, `std::unordered_set`, `std::unordered_multimap`, `std::unordered_map`.

To use as a set (only keys):

```cpp
	Hash_Table<int> s = {11, 22, 33, 44, 55};
	cout << s(34).exists() << endl; // false
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

### `salgo::Hash`

Hash_Table uses `salgo::Hash<Key>` by default to get hashes for objects.

The `salgo::Hash<Key>` first checks the `Key` type for member `hash()` function, and uses it if available. Otherwise, it uses `std::hash<Key>`.

If you want to provide a hashing function for your custom type, don't specialize `salgo::Hash`, but either:
* Create a member `hash()` function, or
* Specialize `std::hash<Key>` instead

### Type Builder

#### ::COUNT

Add a counter so total of number elements can be checked using `count()`.

#### ::EXTERNAL

By default, elements are stored inplace. Use `::EXTERNAL` to store handles to elements allocated using Allocator instead.

#### ::ALLOCATOR<Alloc>

Allocator used to construct elements. Used only if `::EXTERNAL`.


### Performance (x86_64)

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


### TODO
* Implement not-multi version, e.g. by `::UNIQUE` type parameter.
* Expose parameter for bucket INPLACE_BUFFER
* implement initializer lists, actually...



List
----
Similar to `std::list`.



### Performance (x86_64)

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






Exception safety
================
No such thing. Google doesn't care, neither do I.

