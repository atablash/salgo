#pragma once


namespace salgo {

namespace internal {
namespace key_val {


	template<class Key, class Val>
	struct Key_Val {
		const Key key; // const_cast'ed when moving-out values
		Val val;

		// first argument goes to KEY, the rest goes to VAL
		template<class K, class... V>
		Key_Val(K&& k, V&&... v) :
			key( std::forward<K>(k) ),
			val( std::forward<V>(v)... ) {}

		Key_Val(const Key_Val&) = default;

		// Key_Val(Key_Val&&) = default;
		template<class K = Key, class = std::enable_if_t<std::is_move_constructible_v<K>>>
		Key_Val(Key_Val&& o) : key(std::move(*const_cast<Key*>(&o.key))), val(std::move(o.val)) {}

		auto& operator()()       { return *this; }
		auto& operator()() const { return *this; }
	};


	template<class Key>
	struct Key_Val<Key,void> {
		const Key key; // const_cast'ed when moving-out values
		FORWARDING_CONSTRUCTOR_VAR(Key_Val, key) {}

		Key_Val(const Key_Val&) = default;

		// Key_Val(Key_Val&& o) = default;
		template<class K = Key, class = std::enable_if_t<std::is_move_constructible_v<K>>>
		Key_Val(Key_Val&& o) : key(std::move(*const_cast<Key*>(&o.key))) {}

		auto& operator()()       { return key; }
		auto& operator()() const { return key; }
	};


	template<class Val>
	struct Key_Val<void,Val> {
		Val val;
		FORWARDING_CONSTRUCTOR_VAR(Key_Val, val) {}

		Key_Val(const Key_Val&) = default;
		Key_Val(Key_Val&&) = default;

		auto& operator()()       { return val; }
		auto& operator()() const { return val; }
	};

	template<>
	struct Key_Val<void,void> {
		auto& operator()()       { return *this; }
		auto& operator()() const { return *this; }
	};


} // namespace key_val

} // namespace internal


//
// Key can be void
// Val can be void
//
template<class Key, class Val>
using Key_Val = internal::key_val::Key_Val<Key,Val>;



} // namespace salgo
