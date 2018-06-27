#pragma once


namespace salgo {

namespace internal {
namespace key_val {


	template<class Key, class Val>
	struct Key_Val {
		const Key key;
		Val val;

		// first argument goes to KEY, the rest goes to VAL
		template<class K, class... V>
		Key_Val(K&& k, V&&... v) :
			key( std::forward<K>(k) ),
			val( std::forward<V>(v)... ) {}

		auto& operator()()       { return *this; }
		auto& operator()() const { return *this; }
	};

	template<class Key>
	struct Key_Val<Key,void> {
		const Key key;
		FORWARDING_CONSTRUCTOR(Key_Val, key) {}

		auto& operator()()       { return key; }
		auto& operator()() const { return key; }
	};

	template<class Val>
	struct Key_Val<void,Val> {
		Val val;
		FORWARDING_CONSTRUCTOR(Key_Val, val) {}

		auto& operator()()       { return val; }
		auto& operator()() const { return val; }
	};

	template<>
	struct Key_Val<void,void> {};


} // namespace key_val

} // namespace internal


//
// Key can be void
// Val can be void
//
template<class Key, class Val>
using Key_Val = internal::key_val::Key_Val<Key,Val>;



} // namespace salgo
