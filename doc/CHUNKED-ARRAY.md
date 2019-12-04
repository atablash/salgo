Chunked_Array
=============
It's similar to `Dynamic_Array`, but doesn't reallocate. Instead, it creates new memory blocks for the new elements, so it's not contiguous.

You can use it when you have non-movable elements (same as with `std::deque`).

It's used internally in `Random_Allocator`.




Loops and erasing elements
--------------------------
Safe through both the Accessor object or Chunked_Array object, same as with `salgo::Dynamic_Array`.




Accessors and handles invalidation
----------------------------------
Similarly to `salgo::Dynamic_Array`, accessors/iterators and handles never invalidate, except when the `Chunked_Array` object is moved.

Pointers to elements never invalidate, because objects are never moved.


