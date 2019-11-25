#pragma once

#include "common.hpp"

#include <functional>


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




//
// tuples interface (maybe can be removed in favor of automatic accessible members structured binding)
//
#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-tags"
#endif
template<class K, class V>
struct std::tuple_size<salgo::internal::key_val::Key_Val<K,V>>    : std::integral_constant<std::size_t, 2> {};

template<class K>
struct std::tuple_size<salgo::internal::key_val::Key_Val<K,void>> : std::integral_constant<std::size_t, 1> {};

template<class V>
struct std::tuple_size<salgo::internal::key_val::Key_Val<void,V>> : std::integral_constant<std::size_t, 1> {};

template<>
struct std::tuple_size<salgo::internal::key_val::Key_Val<void,void>> : std::integral_constant<std::size_t, 0> {};
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif


template<std::size_t ITH, class K, class V>
struct std::tuple_element<ITH, salgo::internal::key_val::Key_Val<K,V>>    { static_assert(ITH == 0 || ITH == 1); using type = std::conditional_t<ITH == 0, const K, V>; };

template<std::size_t ITH, class K>
struct std::tuple_element<ITH, salgo::internal::key_val::Key_Val<K,void>> { static_assert(ITH == 0); using type = const K; };

template<std::size_t ITH, class V>
struct std::tuple_element<ITH, salgo::internal::key_val::Key_Val<void,V>> { static_assert(ITH == 0); using type = V; };


namespace std {
	template<std::size_t ith, class K, class V>
	constexpr auto& get(      salgo::internal::key_val::Key_Val<K, V>& kv) { static_assert(0 <= ith && ith < 2); if constexpr(ith == 0) return kv.key; else return kv.val; }

	template<std::size_t ith, class K, class V>
	constexpr auto& get(const salgo::internal::key_val::Key_Val<K, V>& kv) { static_assert(0 <= ith && ith < 2); if constexpr(ith == 0) return kv.key; else return kv.val; }


	template<std::size_t ith, class K>
	constexpr auto& get(      salgo::internal::key_val::Key_Val<K, void>& kv) { static_assert(ith == 0); return kv.key; }

	template<std::size_t ith, class K>
	constexpr auto& get(const salgo::internal::key_val::Key_Val<K, void>& kv) { static_assert(ith == 0); return kv.key; }


	template<std::size_t ith, class V>
	constexpr auto& get(      salgo::internal::key_val::Key_Val<void, V>& kv) { static_assert(ith == 0); return kv.value; }

	template<std::size_t ith, class V>
	constexpr auto& get(const salgo::internal::key_val::Key_Val<void, V>& kv) { static_assert(ith == 0); return kv.value; }
}
