#pragma once

#include "common.hpp"
#include "const-flag.hpp"

#include <glog/logging.h>




namespace salgo {
namespace internal {


template<Const_Flag C, class CONTEXT> class Reference;
template<Const_Flag C, class CONTEXT> class Accessor_Base;
template<Const_Flag C, class CONTEXT> class Iterator_Base;



GENERATE_HAS_MEMBER(Extra_Context)

template<bool, class> struct Extra_Context {};
template<class CONTEXT> struct Extra_Context<true,CONTEXT> : CONTEXT::Extra_Context {};


template<Const_Flag C, class CONTEXT>
class Reference : protected Extra_Context<has_member_Extra_Context<CONTEXT>::value, CONTEXT> {
public:
	using Container = typename CONTEXT::Container;
	using Handle = typename CONTEXT::Handle;

public:
	Reference(Const<Container,C>* container, Handle handle) : _handle(handle), _container(container) {}

protected:
	Handle _handle;
	Const<Container,C>* _container;
};





template<Const_Flag C, class CONTEXT>
class Accessor_Base : public Reference<C,CONTEXT> {
	using BASE = Reference<C,CONTEXT>;

	template<Const_Flag CC> using Accessor = typename CONTEXT::template Accessor<CC>;
	template<Const_Flag CC> using Iterator = typename CONTEXT::template Iterator<CC>;
	using Handle = typename CONTEXT::Handle;

public:
	FORWARDING_CONSTRUCTOR(Accessor_Base, BASE) {}

	// get handle
	auto     handle() const { return BASE::_handle; }
	operator Handle() const { return handle(); }

	// get value
	auto& operator()()       { return (*BASE::_container)[ BASE::_handle ]; }
	auto& operator()() const { return (*BASE::_container)[ BASE::_handle ]; }
	operator       auto&()       { return operator()(); }
	operator const auto&() const { return operator()(); }

	template<class VAL>
	auto& operator=(VAL&& val) { operator()() = std::forward<VAL>(val); return _self(); }


private:
	auto _base()       { return static_cast<      BASE*>(this); }
	auto _base() const { return static_cast<const BASE*>(this); }

	void _check() const {
		static_assert(sizeof(Accessor<C>) == sizeof(BASE), "Accessors can't have any additional members");
		static_assert(sizeof(Iterator<C>) == sizeof(BASE), "Accessors can't have any additional members");
	}

public:
	auto& iterator()       { _check(); return *static_cast<      Iterator<C>*>(_base()); }
	auto& iterator() const { _check(); return *static_cast<const Iterator<C>*>(_base()); }


	CRTP_COMMON( Iterator_Base, Accessor<C> )
};






GENERATE_HAS_MEMBER(_will_compare_with);

template<Const_Flag C, class CONTEXT>
class Iterator_Base : public Reference<C,CONTEXT> {
	using BASE = Reference<C,CONTEXT>;

	template<Const_Flag CC> using Accessor = typename CONTEXT::template Accessor<CC>;
	template<Const_Flag CC> using Iterator = typename CONTEXT::template Iterator<CC>;

public:
	FORWARDING_CONSTRUCTOR(Iterator_Base, BASE) {}


public:
	Iterator_Base() = default;
	Iterator_Base(const Iterator_Base&) = delete;
	Iterator_Base(Iterator_Base&&) = default;
	Iterator_Base& operator=(const Iterator_Base&) = delete;
	Iterator_Base& operator=(Iterator_Base&&) = delete;

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
	bool operator==(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _self()._get_comparable() == o._get_comparable();
	}

	template<Const_Flag CC>
	bool operator!=(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _self()._get_comparable() != o._get_comparable();
	}

	template<Const_Flag CC>
	bool operator<(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _self()._get_comparable() < o._get_comparable();
	}

	template<Const_Flag CC>
	bool operator>(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _self()._get_comparable() > o._get_comparable();
	}

	template<Const_Flag CC>
	bool operator<=(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _self()._get_comparable() <= o._get_comparable();
	}

	template<Const_Flag CC>
	bool operator>=(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _self()._get_comparable() >= o._get_comparable();
	}


private:
	template<Const_Flag CC>
	void _will_compare_with_base(const Iterator<CC>& o) const {
		DCHECK_EQ(BASE::_container, o._container) << "comparing iterators to different containers";
		if constexpr(has_member__will_compare_with< Iterator<C> >::value) {
			_self()._will_compare_with(o);
		}
	}


private:
	auto _base()       { return static_cast<      BASE*>(this); }
	auto _base() const { return static_cast<const BASE*>(this); }

	void _check() const {
		static_assert(sizeof(Accessor<C>) == sizeof(BASE), "Iterators can't have any additional members");
		static_assert(sizeof(Iterator<C>) == sizeof(BASE), "Iterators can't have any additional members");
	}

public:
	auto& accessor()       { _check(); return *static_cast<      Accessor<C>*>(_base()); }
	auto& accessor() const { _check(); return *static_cast<const Accessor<C>*>(_base()); }

	// return accessor
	auto& operator*()       { return accessor(); }
	auto& operator*() const { return accessor(); }
	auto operator->()       { return &accessor(); }
	auto operator->() const { return &accessor(); }


	CRTP_COMMON( Iterator_Base, Iterator<C> )
};



} // namespace internal
} // namespace salgo

