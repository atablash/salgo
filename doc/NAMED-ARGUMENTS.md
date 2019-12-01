Named_Arguments
===============
Define optional named arguments for functions.
* Supply arguments in any order
* Full support for move semantics
* Conditionally forward arguments to nested function calls
* Theoretically should not produce any runtime overhead (TODO: check disassembly)
* Each called function signature is compiled as a separate template instantiation, giving compiler field for optimizations

Basic Usage
-----------

```cpp
	#include <salgo/named-arguments>

	NAMED_ARGUMENT(ALPHA)
	NAMED_ARGUMENT(BETA)
	NAMED_ARGUMENT(GAMMA)

	using namespace salgo;

	template<class... ARGS>
	void fun(int some_fixed_arg, ARGS&&... _args) {
		auto args = Named_Arguments( std::forward<ARGS>(_args)... );

		std::cout << some_fixed_arg << std::endl; // 666

		std::cout << args(ALPHA) << std::endl; // 0.5
		std::cout << args(ALPHA, "default to this c-string") << std::endl; // 0.5 (not using default c-string value)

		// std::cout << args(BETA) << std::endl; // compile error
		std::cout << args(BETA, 123) << std::endl; // 123 (no BETA, defaulting to 123)

		std::cout << args.has(BETA) << std::endl; // 0 (false)
	}

	fun(666, GAMMA="test", ALPHA=0.5);
```

Non-const References
--------------------
By default, every argument is passed as *const lvalue-reference* (`const T&`).

If you want to modify arguments inside the function, a special syntax is needed to signify that you know what you're doing:

```cpp
	fun( ALPHA << non_const_ref, GAMMA = regular_const_ref )
```

> NOTE
>
> This is in a way more expressive than regular function calls, where you don't mark arguments that are forbidden to be modified inside the function. (only the other way around)




Passing Arguments Further
-------------------------
Arguments can be passed to nested functions that also take named arguments.

Arguments can be forwarded as:
* *perfect-forwarded* - using `PARAM = args.forward()` - either `T&`, or `T&&`, without change
* *lvalue-references* - using `PARAM = args` - will forbid *moving-out* of the argument, i.e. always use `T&`


```cpp
	#include <salgo/named-arguments>

	NAMED_ARGUMENT(ALPHA)
	NAMED_ARGUMENT(BETA)
	NAMED_ARGUMENT(GAMMA)

	using namespace salgo;

	template<class... ARGS>
	void another_fun(ARGS&&... _args) {
		auto args = Named_Arguments( std::forward<ARGS>(_args)... );

		auto g = args(GAMMA); // move-constructs `unique_ptr`
		// auto a = args(ALPHA); // does not compile
	}

	template<class... ARGS>
	void fun(ARGS&&... _args) {
		auto args = Named_Arguments( std::forward<ARGS>(_args)... );
		another_fun(
			ALPHA << args,
			GAMMA << args.forward(),
			BETA  << args // no effect (no BETA supplied)
		);
	}

	fun(666, GAMMA << std::make_unique<int>(2424), ALPHA << std::make_unique<int>(1313));
```

This special syntax for passing arguments further has some advantages over doing simply `ALPHA = args(ALPHA)`:
* You can pass non-supplied arguments, it will have no effect.
* `ALPHA = args` is shorter than `ALPHA = args(ALPHA)`
* There wouldn't be a simple way to *perfect-forward* arguments, if not for `ALPHA = args.forward()`

> NOTE
>
> Neither `PARAM << args` or `PARAM << args.forward()` does invoke any copy or move construction.
>
> What they do instead, is *static-cast* to `T&` and *perfect-forward*, respectively.
>
> `PARAM = arg` syntax also adds *const-qualifier* to the passed reference.




Protect Yourself
----------------
Use the most strict construct available:
* Prefer `PARAM = value` over `PARAM << value` (adds *const-qualifier*)
* Prefer `PARAM << args` over `PARAM << args.forward()` (forces *rvalue-reference* into *lvalue-reference*)

The simplest syntax has been chosen to be the most strict one, so it should be straightforward to do the right thing.




More Examples
-------------
See `test/named-arguments.cpp` for more usage examples.


