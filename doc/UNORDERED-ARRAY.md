Unordered_Array
===============
It's a small wrapper around `Dynamic_Array`. When erasing an element, the last element in the array is moved to fill the hole.

### Loops and erasing elements

Erasing during backward iteration is always safe.

Erasing during forward iteration is safe only through iterating Accessor, but not though the `Unordered_Array` object, i.e.:

```cpp
	Unordered_Array v = {1, 2, 3, 4, 5};
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




Performance
-----------
Even though forward iteration requires a check if element was just deleted or not (as opposed to backward iteration), there's no overhead as measured in benchmarks, and both forward and backward iteration have the same performance. Backward iteration through salgo::Reversed adapter also has no overhead.



See Also
--------
* [Dynamic_Array](DYNAMIC-ARRAY.md) - a replacement for `std::vector`
