#pragma once

#include "../accessors.hpp"
#include "../const-flag.hpp"

#include "../helper-macros-on"

namespace salgo {
namespace internal {
namespace n_ary_forest {




template<class P>
struct Context;




template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C, Context<P>>, public P {
	using BASE = Accessor_Base<C, Context<P>>;

	using P::N_Ary;
	using typename P::Handle;
	using typename P::Handle_Small;

public:
	using BASE::BASE;
	// friend Rooted_Forest;
	friend Accessor<P, CONST>;
	friend Accessor<P, MUTAB>;

public:
	auto& key()       { return NODE.key; }
	auto& key() const { return NODE.key; }

	auto& val()       { return NODE.val; }
	auto& val() const { return NODE.val; }

public:
	bool exists() const { return HANDLE.valid(); }


	auto child(int ith)       { _check_child_index(ith);
			return Accessor<P, C    >( &CONT, BASE::get_children()[ith] ); }

	auto child(int ith) const { _check_child_index(ith);
			return Accessor<P, CONST>( &CONT, BASE::get_children()[ith] ); }

	auto left()        { static_assert(N_Ary == 2); return child(0); }
	auto left()  const { static_assert(N_Ary == 2); return child(0); }

	auto right()       { static_assert(N_Ary == 2); return child(1); }
	auto right() const { static_assert(N_Ary == 2); return child(1); }


	auto has_left()        { static_assert(N_Ary == 2); return child(0).exists(); }
	auto has_left()  const { static_assert(N_Ary == 2); return child(0).exists(); }

	auto has_right()       { static_assert(N_Ary == 2); return child(1).exists(); }
	auto has_right() const { static_assert(N_Ary == 2); return child(1).exists(); }


	auto parent()       { return Accessor<P, C    >( &CONT, BASE::get_parent() ); }
	auto parent() const { return Accessor<P, CONST>( &CONT, BASE::get_parent() ); }



	bool is_ith_child(int ith) const {
		auto par = BASE::get_parent(); DCHECK(par.valid());
		parent()._check_child_index(ith);
		return HANDLE == _node(par).children[ith];
	}

	auto& is_which() const {
		auto par = BASE::get_parent(); DCHECK(par.valid());
		for(auto& ch : _node(par).children) {
			if(ch == BASE::HANDLE) return ch;
		}
		DCHECK(false);
		return _node(par).children[0]; // just to supress warning
	}

	bool is_left() const {
		static_assert(N_Ary == 2);
		return is_ith_child(0);
	}

	bool is_right() const {
		static_assert(N_Ary == 2);
		return is_ith_child(0);
	}



	template<class... Args>
	auto emplace_child(int ith, Args&&... args) {
		static_assert(C == MUTAB, "called on CONST accessor");
		DCHECK( !child(ith).exists() );
		auto new_node = ALLOC.construct( std::forward<Args>(args)... );
		link_child(ith, new_node);
		return child(ith);
	}

	template<class... Args>
	auto emplace_left (Args&&... args) { static_assert(N_Ary == 2); return emplace_child(0, std::forward<Args>(args)...); }

	template<class... Args>
	auto emplace_right(Args&&... args) { static_assert(N_Ary == 2); return emplace_child(1, std::forward<Args>(args)...); }


	void link_child(int ith, Handle new_child) {
		static_assert(C == MUTAB, "called on CONST accessor");
		DCHECK( exists() );
		_check_child_index(ith);

		if constexpr(P::Has_Child_Links) {
			DCHECK( !child(ith).exists() ); // no child
			NODE.children[ith] = new_child;
		}

		if constexpr(P::Has_Parent_Links) {
			DCHECK( !CONT(new_child).parent().exists() ); // new_child has no parent
			_node(new_child).parent = HANDLE;
		}
	}

	void link_left (Handle new_child) { static_assert(N_Ary == 2); link_child(0, new_child); }
	void link_right(Handle new_child) { static_assert(N_Ary == 2); link_child(1, new_child); }


	// automatically unlink things if needed (slower)
	void relink_child(int ith, Handle new_child) {
		if(_node(new_child).parent.valid()) BASE::CONT(new_child).unlink_parent();
		if(NODE.children[ith].valid()) unlink_child(ith);

		link_child(ith, new_child);
	}



private:
	void _unlink_child_1way(Handle_Small& ch) {
		static_assert(C == MUTAB, "called on CONST accessor");
		DCHECK( exists() );

		DCHECK( ch.valid() );
		_node(ch).parent.reset();
	}

public:
	void unlink_child(int ith) {
		static_assert(C == MUTAB, "called on CONST accessor");
		DCHECK( exists() );

		auto& ch = _node().children[ith];
		_unlink_child_1way(ch);
		ch.reset();
	}

	void unlink_left()  { unlink_child(0); }
	void unlink_right() { unlink_child(1); }



private:
	// TODO: NOTE: linear in the number of children!
	void _unlink_parent_1way() {
		DCHECK( exists() );
		auto& par = _node( NODE.parent );
		DCHECK( std::find(par.children.begin(), par.children.end(), HANDLE) != par.children.end() );
		for(auto& ch : par.children) if(ch == HANDLE) {
			ch.reset();
			DCHECK_GT(N_Ary, 0) << "not implemented";
			// TODO: don't leave holes when N_ARY==0 (dynamic)
			// maybe use Unordered_Vector or Vector_Allocator instead for children list
			break;
		}
	}
public:
	void unlink_parent() {
		_unlink_parent_1way();
		_node().parent.reset();
	}


private:
	// prerequisite: no element points to this element (not checked)
	// when unlink_and_erase was just called, links will be 1-way
	void _erase_unchecked() {
		static_assert(C == MUTAB, "called on CONST accessor");
		DCHECK( exists() );
		BASE::on_erase(); // cache links before removing this node
		ALLOC( HANDLE ).destruct();
	}

public:
	// PREREQUISITE: does not have parent or child
	void erase() {
		static_assert(C == MUTAB, "called on CONST accessor");
		DCHECK( exists() );
		DCHECK( !NODE.parent.valid() ) << "can't erase, still has parent";

		for(auto& ch : NODE.children) DCHECK( !ch.valid() ) << "can't erase, still has child";

		_erase_unchecked();
	}

	// unlink both parent and children
	void unlink_and_erase() {
		static_assert(C == MUTAB, "called on CONST accessor");
		DCHECK( exists() );

		// unlink parent (if present)
		if( NODE.parent.valid() ) _unlink_parent_1way();

		// unlink children
		for(auto& ch : NODE.children) {
			if(ch.valid()) _unlink_child_1way(ch);
		}

		_erase_unchecked();
	}


private:
	void _check_child_index(int ith) const {
		DCHECK_GE(ith, 0); DCHECK_LT(ith, BASE::get_children().size());
	}

	//void aggregate() {
	//	static_assert(C == MUTAB, "called aggregate() on CONST accessor");
	//	_node().aggreg = Aggreg();
	//	if() _node().aggreg.aggregate();
	//}

private:
	auto& _node(Handle h)       { return ALLOC[ h ]; }
	auto& _node(Handle h) const { return ALLOC[ h ]; }
};




} // namespace n_ary_forest
} // namespace internal
} // namespace salgo

#include "../helper-macros-off"
