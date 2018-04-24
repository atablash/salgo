#pragma once


namespace salgo {

namespace internal {
namespace key_val {

template<class T> struct Add_val {
	T val;

	template<class TT>
	Add_val(TT&& v) : val( std::forward<TT>(v) ) {}
};
template<> struct Add_val<void> {};


template<class Key, class Val>
struct Key_Val : Add_val<Val> {
	const Key key;

	// first argument goes to KEY, the rest goes to VAL
	template<class K, class... V>
	Key_Val(K&& k, V&&... v) : Add_val<Val>( std::forward<V>(v)... ), key( std::forward<K>(k) ) {}
};


}

} // namespace internal


//
// Val can be void
//
template<class Key, class Val>
using Key_Val = internal::key_val::Key_Val<Key,Val>;



} // namespace salgo
