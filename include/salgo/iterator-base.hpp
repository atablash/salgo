#pragma once

#include "common.hpp"
#include "const-flag.hpp"

#include <glog/logging.h>

namespace salgo {





GENERATE_HAS_MEMBER(_will_compare_with);



template< Const_Flag C, template<Const_Flag> class CRTP >
class Iterator_Base {

// make polymorphic for debugging with dynamic_case
public:
	#ifndef NDEBUG
	virtual ~Iterator_Base() = default;
	#endif

private:
	auto& _self() {
		DCHECK_EQ(this, dynamic_cast< CRTP<C>* >(this));
		return *reinterpret_cast< CRTP<C>* >(this);
	}

	auto& _self() const {
		DCHECK_EQ(this, dynamic_cast< const CRTP<C>* >(this));
		return *reinterpret_cast< const CRTP<C>* >(this);
	}


public:
	auto& operator++() {
		_self()._increment();
		return *this; }

	auto operator++(int) {
		auto old = *this;
		_self()._increment();
		return old; }

	auto& operator--() {
		_self()._decrement();
		return *this; }

	auto operator--(int) {
		auto old = *this;
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
};




} // namespace salgo


