#pragma once



namespace salgo {




template<class V>
auto bst_next(const V& v) {
	DCHECK(v.exists());

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
			DCHECK(curr.has_parent());
			curr = curr.parent();

			if(curr.is_left()) return curr.parent();
		}
	}
}


template<class V>
auto bst_prev(const V& v) {
	DCHECK(v.exists());

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
			DCHECK(curr.has_parent());
			curr = curr.parent();

			if(curr.is_right()) return curr.parent();
		}
	}
}




// try to cut out node (possible if max 1 child)
template<class V>
bool bst_cut_out(V& v) {
	DCHECK(v.exists());

	if(!v.has_right()) {
		if(!v.has_parent()) return true;

		auto parent = v.parent();
		auto ith = v.is_which();

		if(v.has_left()) parent.link_child(ith, v.left());
	}
	else if(!v.has_left()) {
		if(!v.has_parent()) return true;

		auto parent = v.parent();
		auto ith = v.is_which();

		if(v.has_right()) parent.link_child(ith, v.right());
	}
	else {
		return false;
	}

	return true;
}





// remove node and replace the hole with _next
template<class V>
void bst_erase(V& v) {
	DCHECK(v.exists());

	if( bst_cut_out(v) ) {
		v.erase();
		return;
	}

	auto next = bst_next(v);

	v() = next();

	// replace with _next
	bool r = bst_cut_out(next);
	DCHECK(r);
	next.erase();
}




} // namespace salgo


