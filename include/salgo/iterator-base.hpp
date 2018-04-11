#pragma once

#include "common.hpp"
#include "const-flag.hpp"

#include <glog/logging.h>

namespace salgo {





GENERATE_HAS_MEMBER(_will_compare_with);



template< Const_Flag C, template<Const_Flag> class CRTP >
class Iterator_Base {


//
// required to be std::iterator-like
//
public:
	using difference_type = ptrdiff_t;
	using value_type = char;
	using pointer = value_type*;
	using reference = value_type&;
	using iterator_category = std::bidirectional_iterator_tag;


public:
	auto& operator++() {
		_self()._increment();
		return _self(); }

	auto operator++(int) {
		auto old = _self();
		_self()._increment();
		return old; }

	auto& operator--() {
		_self()._decrement();
		return _self(); }

	auto operator--(int) {
		auto old = _self();
		_self()._decrement();
		return old; }


public:
	template<Const_Flag CC>
	bool operator==(const CRTP<CC>& o) const {
		_will_compare_with(o);
		return _self()._get_comparable() == o._self()._get_comparable();
	}

	template<Const_Flag CC>
	bool operator!=(const CRTP<CC>& o) const {
		_will_compare_with(o);
		return _self()._get_comparable() != o._self()._get_comparable();
	}

	template<Const_Flag CC>
	bool operator<(const CRTP<CC>& o) const {
		_will_compare_with(o);
		return _self()._get_comparable() < o._self()._get_comparable();
	}

	template<Const_Flag CC>
	bool operator>(const CRTP<CC>& o) const {
		_will_compare_with(o);
		return _self()._get_comparable() > o._self()._get_comparable();
	}

	template<Const_Flag CC>
	bool operator<=(const CRTP<CC>& o) const {
		_will_compare_with(o);
		return _self()._get_comparable() <= o._self()._get_comparable();
	}

	template<Const_Flag CC>
	bool operator>=(const CRTP<CC>& o) const {
		_will_compare_with(o);
		return _self()._get_comparable() >= o._self()._get_comparable();
	}


private:
	template<Const_Flag CC>
	void _will_compare_with(const CRTP<CC>& o) const {
		if constexpr(has_member__will_compare_with< CRTP<C> >::value) {
			_self()._will_compare_with(o);
		}
	}



public:
	// return this (iterator == accessor)
	auto& operator*()       { return _self(); }
	auto& operator*() const { return _self(); }

	// get accessor's val()
	auto operator->()       { return &_self()(); }
	auto operator->() const { return &_self()(); }


private:
	auto& _self()       { _check_crtp(); return *reinterpret_cast<       CRTP<C>* >(this); }
	auto& _self() const { _check_crtp(); return *reinterpret_cast< const CRTP<C>* >(this); }


	#ifndef NDEBUG
		// make polymorphic for debugging with dynamic_cast
		public:	virtual ~Iterator_Base() = default;
		private: void _check_crtp() const { DCHECK(dynamic_cast<const CRTP<C>*>(this)); }
	#else
		private: void _check_crtp() const {}
	#endif

};




} // namespace salgo


