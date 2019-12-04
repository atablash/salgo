#pragma once

#include <glog/logging.h>

namespace salgo {




template<class V>
auto bst_next(const V& v) {
	DCHECK(v.valid());

	auto curr = v;

	if(curr.has_right()) {
		// go 1 right and n left
		curr = curr.right();
		while(curr.has_left()) curr = curr.left();
		return curr;
	}
	else {
		// go n up-left and 1 up-right
		for(;;) {
			//DCHECK(curr.has_parent());
			if(curr.has_no_parent() || curr.is_left()) return curr.parent(); 
			curr = curr.parent();
		}
	}
}


template<class V>
auto bst_prev(const V& v) {
	DCHECK(v.valid());

	auto curr = v;

	if(curr.has_left()) {
		// go 1 left and n right
		curr = curr.left();
		while(curr.has_right()) curr = curr.right();
		return curr;
	}
	else {
		// go n up-right and 1 up-left
		for(;;) {
			//DCHECK(curr.has_parent());
			if(!curr.parent().handle().valid() || curr.is_right()) return curr.parent(); 
			curr = curr.parent();
		}
	}
}




// try to cut out node (possible if max 1 child)
template<class V>
bool bst_cut_out(V& v) {
	DCHECK(v.valid());

	if(v.has_no_right()) {
		if(v.has_no_parent()) {
			v.unlink_and_erase(); // optim?
			return true;
		}

		auto parent = v.parent();
		auto ch = v.is_which();

		if(v.has_left()) parent.relink_child(ch, v.left());
	}
	else if(!v.has_left()) {
		if(v.has_no_parent()) {
			v.unlink_and_erase(); // optim?
			return true;
		}

		auto parent = v.parent();
		auto ch = v.is_which();

		if(v.has_right()) parent.relink_child(ch, v.right());
	}
	else {
		return false;
	}

	v.unlink_and_erase();
	return true;
}





// remove node and replace the hole with prev element
template<class V>
void bst_erase(V& v) {
	DCHECK(v.valid());

	if( bst_cut_out(v) ) return;

	auto prv = bst_prev(v);

	v() = std::move( prv() ); // move value

	// replace with prev
	bool r = bst_cut_out(prv);
	DCHECK(r);
}




} // namespace salgo


