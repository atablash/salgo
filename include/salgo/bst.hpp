#pragma once


namespace salgo {



//
// operations on bst tree nodes
//
template<class ALLOC, class HANDLE>
inline auto bst_next(const ALLOC& alloc, HANDLE handle) {
	DCHECK(handle.valid());

	if(alloc[handle].right.valid()) {
		// go 1 right and n left
		handle = alloc[handle].right;
		while(alloc[handle].left.valid()) handle = alloc[handle].left;
		return handle;
	}
	else {
		// go n up and 1 right
		for(;;) {
			HANDLE parent = alloc[handle].parent;
			DCHECK(parent.valid());
			if(alloc[parent].left == handle) return parent;
			handle = parent;
		}
	}
}

template<class ALLOC, class HANDLE>
inline auto bst_prev(const ALLOC& alloc, HANDLE handle) {
	DCHECK(handle.valid());

	if(alloc[handle].left.valid()) {
		// go 1 left and n right
		handle = alloc[handle].left;
		while(alloc[handle].right.valid()) handle = alloc[handle].right;
		return handle;
	}
	else {
		// go n up and 1 left
		for(;;) {
			HANDLE parent = alloc[handle].parent;
			DCHECK(parent.valid());
			if(alloc[parent].right == handle) return parent;
			handle = parent;
		}
	}
}


//
// add (key,val) to a BST, return handle to the new node
//
template<class ALLOC, class HANDLE, class K, class... V>
inline auto bst_emplace(const ALLOC& alloc, HANDLE handle, K&& k, V&&... v) {
	for(;;) {
		if(k >= alloc[handle].key) {
			if(!alloc[handle].right.valid()) {
				auto new_node = alloc.construct( std::forward<K>(k), std::forward<V>(v)... ).handle();
				alloc[handle].right = new_node;
				alloc[new_node].parent = handle;
				return new_node;
			}
			handle = alloc[handle].right;
		}
		else {
			if(!alloc[handle].left.valid()) {
				auto new_node = alloc.construct( std::forward<K>(k), std::forward<V>(v)... ).handle();
				alloc[handle].left = new_node;
				alloc[new_node].parent = handle;
				return new_node;
			}
			handle = alloc[handle].left;
		}
	}
}




} // namespace salgo


