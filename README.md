[![Build Status](https://travis-ci.org/atablash/salgo.svg?branch=master)](https://travis-ci.org/atablash/salgo)
[![codecov](https://codecov.io/gh/atablash/salgo/branch/master/graph/badge.svg)](https://codecov.io/gh/atablash/salgo)
[![Coverage Status](https://coveralls.io/repos/github/atablash/salgo/badge.svg?branch=master)](https://coveralls.io/github/atablash/salgo?branch=master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/6bb9fedab6e642b6b80e85cc4b48845a)](https://www.codacy.com/manual/atablash/salgo?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=atablash/salgo&amp;utm_campaign=Badge_Grade)
[![CodeFactor](https://www.codefactor.io/repository/github/atablash/salgo/badge/master)](https://www.codefactor.io/repository/github/atablash/salgo/overview/master)



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


Contents
========
* [Library Design](#library-design)
	* [Accessors](#accessors)
	* [Handles](#handles)
	* [Iterators](#iterators)
	* [Allocators](#allocators)
	* [A Story Behind Accessors](#a-story-behind-accessors)
	* [Type Builders](#type-builders)
* [Exception Safety](#exception-safety)
* [Reference](#reference)
	* Containers
		* [Vector](doc/VECTOR.md) - a replacement for `std::vector`
		* [Memory_Block](doc/MEMORY-BLOCK.md) - similar to `Vector`, but without automatic grow
		* [Chunked_Vector](doc/CHUNKED-VECTOR.md)
		* [Hash_Table](doc/HASH-TABLE.md) - a replacement for `std::map` and `std::set`
		* [List](doc/LIST.md) - a replacement for `std::list`
	* Data Structures
		* Union_Find - TODO, but see tests
		* Graph - TODO, but see tests
		* Rooted_Forest - TODO, but see tests
	* 3D
		* TODO
	* Allocators
		* [Vector_Allocator](doc/VECTOR-ALLOCATOR.md) - the default Salgo allocator
		* [Random_Allocator](doc/RANDOM-ALLOCATOR.md)
		* [Crude_Allocator](doc/CRUDE-ALLOCATOR.md)
		* [Salgo_From_Std_Allocator](doc/SALGO-FROM-STD-ALLOCATOR.md) - adapter for `std` compatible allocators
	* Other
		* Modulo - TODO, but see tests
		* Binomial - TODO, but see tests
		* Stack_Storage - TODO, but see tests



Library Design
=================
The library is contained within `salgo` namespace, so either type `salgo::` explicitly each time (especially in header files or in case of name conflicts), or just use:

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
	struct Obj {
		int foo;

		Obj(int x) : foo(x) {}
	};

	Vector<Obj> v = {6, 24, 35, 46, 57};

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
> Often there's also a `Handle_Small` available, which is a compressed version, meant for storage.
>
> For node-based structures like `salgo::List`, a `Handle_Small` is currently 4 bytes, and `Handle` is 8 bytes.
>
> Use `Handle_Small` only for storage. It has some overhead to decode it back.
>
> There are conversions defined both ways (currently implicit).




Indexes
-------
Some structures also expose `Index` type, which is usually same as `Handle`, but can be implicitly created from `int`.

For example, `Vector_Allocator` is natually indexed by `int`, so it exposes functions that accept `Index` type instead of `Handle`.
This way, these functions happily accept `int` parameters, while still accepting `Handle`, which is implicitly convertible to `Index`.




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

Just like accessor, for simplicity, iterator is implicitly convertible to both underlying object reference and *Handle*. To get this explicit, use `operator()()` and `handle()` (again, same as with accessor).




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




Exception safety
================
No such thing. Google doesn't care, neither do I.




Reference
=========
Apart from the following reference, you can also read the code in `test` directory to learn how to use Salgo from unit tests.

* Containers
	* [Vector](doc/VECTOR.md) - a replacement for `std::vector`
	* [Memory_Block](doc/MEMORY-BLOCK.md) - similar to `Vector`, but without automatic grow
	* [Chunked_Vector](doc/CHUNKED-VECTOR.md)
	* [Hash_Table](doc/HASH-TABLE.md) - a replacement for `std::map` and `std::set`
	* [List](doc/LIST.md) - a replacement for `std::list`
* Allocators
	* [Vector_Allocator](doc/VECTOR-ALLOCATOR.md) - the default Salgo allocator
	* [Random_Allocator](doc/RANDOM-ALLOCATOR.md)
	* [Crude_Allocator](doc/CRUDE-ALLOCATOR.md)
	* [Salgo_From_Std_Allocator](doc/SALGO-FROM-STD-ALLOCATOR.md) - adapter for `std` compatible allocators




