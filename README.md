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


As you can see in the above example, accessors are implicitly convertible to the raw type (`int` in this case).

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
	v[3] = 33;		// operator[] returns raw element
	v(3).erase();	// operator() returns accessor
```


### A Story Behind Accessors

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
* It's `CONST`-flavor accessor



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



Handles
-------
If you want to store a reference to an element, use a **handle**.

	Vector<int> v = {11, 22, 33, 44, 55};
	auto h = v(2).handle(); // get a handle

	// ...

	// get an accessor back from the handle:
	v(h).erase(); // and e.g. erase the element

As you can see, a handle doesn't store any context, and you need the owning object (the *Vector* in this case) to get back an accessor.

Handles are meant for storage. Don't try doing this with accessors, they're usually big objects and they're meant to be optimized out.

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

Also enables `::EXISTS`.

> NOTE
>
> If you want just sparse, but not `::EXISTS`, use `::SPARSE_NO_EXISTS` instead, but it'll work only for trivially move constructible types (implies trivially destructible as well).


#### ::EXISTS (default on if ::SPARSE)

For sparse vectors, makes the *Vector* aware of which elements are existing.

This allows resizing (move construction on reallocation), iteration and destruction of elements (if not trivially destructible).

	Vector<int> ::SPARSE v = {1, 2, 33, 4, 5};
	for(auto& e : v) if(e > 10) e.erase();
	// now {1, 2, _, 4, 5}

	for(const auto& e : v) cout << e << ', '; // 1, 2, 4, 5

> NOTE
>
> By default, it keeps `exists` flags in a `std::bitset`-like structure.
>
> You can also choose to keep the flags in-place with the elements, if it works faster for you (probably won't).
>
> You can choose from these two using `::EXISTS_BITSET` or `::EXISTS_INPLACE`. If so, don't include the `::EXISTS` selector!
>
> `::EXISTS` maps to `::EXISTS_BITSET`.


#### ::COUNT

Adds a counter, so makes the *Vector* aware of how many existing elements it has (in constant asymptotic time).

	Vector<int> ::SPARSE ::COUNT v = {1, 2, 3, 4, 5};
	v(2).erase();
	cout << "have " << v.count() << " elements" << endl; // have 4 elements


### Construction

	Vector<int> a;    // en empty vector
	Vector<int> b(10);    // a vector of initial size 10, elements default-constructed
	Vector< std::pair<int,double> > c(10,   69, 1.0);    // a vector of initial size 10, all elements {69,1.0}
	Vector<int> d = {1, 2, 3}; // a vector with elements {1,2,3}


### Accessing elements

	Vector<int> ::SPARSE v = {0, 1, 22, 3, 4};
	v[2] = 2;     // operator[] -> by reference
	v(2).erase(); // operator() -> by accessor



### Foreach-erase loops

	Vector<int> ::SPARSE v = {1, 2, 33, 4, 5};
	for(auto& e : v) if(e > 10) e.erase(); // completely legit!

### Accessors and handles invalidation

Both accessors and handles never invalidate.


### TODO
* Auto-shrink option




Exception safety
================
No such thing. Google doesn't care, neither do I.

