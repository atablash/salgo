#pragma once



namespace salgo {



//
// rotate v once
//
// prerequisites:
// * v has parent
// * v, v.parent and v.parent.parent already have the evert flag propagated
//
template<class V>
void rotate(V& v) {
	DCHECK( v.has_parent() );

	auto p = v.parent();
	v.unlink_parent();

	if(p.has_parent()) {
		auto gp = p.parent();
		if(p.is_left()) {
			gp.unlink_left();
			gp.link_left(v);
		}
		else {
			gp.unlink_right();
			gp.link_right(v);
		}
	}

	if(v.is_left()) {
		auto b = v.right();
		v.unlink_right();
		v.link_right(p);
		p.link_left(b);
	}
	else {
		auto b = v.left();
		v.unlink_left();
		v.link_left(p);
		p.link_right(b);
	}
}



//
// splay v to the root
//
template<class V>
inline void splay(V& v) {
	while(v.has_parent()) {
		// zig-zig
		if(v.parent().has_parent() && v.is_left() == v.parent().is_left()) {
			rotate( v.parent() );
			rotate( v );
		}
		else {
			rotate( v );
		}
	}
}



} // namespace salgo


