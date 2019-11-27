#pragma once

#include "common.hpp"
#include "const-flag.hpp"

#include <glog/logging.h>




namespace salgo {
namespace internal {

GENERATE_HAS_MEMBER(Reference)
GENERATE_HAS_MEMBER(Has_Data)
GENERATE_HAS_MEMBER(Comparable)

template<bool, Const_Flag C, class CONTEXT>
class _Reference;

template<Const_Flag C, class CONTEXT>
using Reference = _Reference< has_member__Reference<CONTEXT>, C, CONTEXT>;

template<Const_Flag C, class CONTEXT> class Accessor_Base;
template<Const_Flag C, class CONTEXT> class Iterator_Base;








GENERATE_HAS_MEMBER(operator[], (int), operator_subscript)


template<Const_Flag C, class CONTEXT>
class Reference_Base {
public:
	using Container = typename CONTEXT::Container;
	using Handle = typename CONTEXT::Handle;

	template<Const_Flag CC> using Reference = Reference<C,CONTEXT>;
	template<Const_Flag CC> using Accessor = typename CONTEXT::template Accessor<CC>;
	template<Const_Flag CC> using Iterator = typename CONTEXT::template Iterator<CC>;


public:
	Reference_Base(Const<Container,C>* container, Handle handle) : _handle(handle), _container(container) {}

private:
	Handle _handle;
	Const<Container,C>* _container;

public:
	auto& accessor()       { _check(); return *static_cast<      Accessor<C>*>(this); }
	auto& accessor() const { _check(); return *static_cast<const Accessor<C>*>(this); }

	auto& iterator()       { _check(); return *static_cast<      Iterator<C>*>(this); }
	auto& iterator() const { _check(); return *static_cast<const Iterator<C>*>(this); }

	auto&    handle()  const { return _handle; }
	operator auto&()   const { return handle(); }

	auto& container()       { return *_container; }
	auto& container() const { return *_container; }

	auto& data()       { static_assert(has_member__operator_subscript<Container>,
		"in order to access DATA/VALUE, your container should have operator[](HANDLE) defined"); return (*_container)[_handle]; }

	auto& data() const { static_assert(has_member__operator_subscript<Container>,
		"in order to access DATA/VALUE, your container should have operator[](HANDLE) defined"); return (*_container)[_handle]; }

	// get value
	auto& operator()()       { return data(); }
	auto& operator()() const { return data(); }

private:
	void _check() const {
		static_assert(sizeof(Accessor<C>) == sizeof(Reference<C>), "accessors can't have any additional members");
		static_assert(sizeof(Iterator<C>) == sizeof(Reference<C>), "accessors can't have any additional members");
	}


protected:
	void _will_compare_with(const Reference_Base& o) const {
		DCHECK_EQ(_container, o._container) << "comparing iterators to different containers";
	}

	auto& _mut_handle() { return _handle; }

public:


	operator       auto&()       {
		if constexpr(has_member__Has_Data<CONTEXT>) {
			if constexpr(!CONTEXT::Has_Data) return *(Invalid_Type*)(1);
			else return operator()();
		}
		else return operator()();
	}

	operator const auto&() const {
		if constexpr(has_member__Has_Data<CONTEXT>) {
			if constexpr(!CONTEXT::Has_Data) return *(Invalid_Type*)(1);
			else return operator()();
		}
		else return operator()();
	}

	// is handle non-null
	//explicit operator bool() const { return _handle().valid(); }
	bool exists() const { return handle().valid(); }
};


template<Const_Flag C, class CONTEXT>
std::ostream& operator<<(std::ostream& s, const Reference_Base<C,CONTEXT>& r) {
	return s << r.handle() << " @" << &r.container();
};
















// derive from CONTEXT::Reference
template<bool, Const_Flag C, class CONTEXT>
class _Reference : public CONTEXT::template Reference<C> {
	using BASE = typename CONTEXT::template Reference<C>;

public:
	using BASE::BASE;
	//FORWARDING_CONSTRUCTOR(_Reference, BASE) {}
};


// ...or deliver straight from Reference_Base, if CONTEXT::Reference is not present
template<Const_Flag C, class CONTEXT>
class _Reference<false,C,CONTEXT> : public Reference_Base<C, CONTEXT> {
	using BASE = Reference_Base<C, CONTEXT>;

public:
	using BASE::BASE;
	//FORWARDING_CONSTRUCTOR(_Reference, BASE) {}
};



template<Const_Flag C, class CONTEXT>
using Reference = _Reference< has_member__Reference<CONTEXT>, C, CONTEXT>;















template<Const_Flag C, class CONTEXT>
class Accessor_Base : public Reference<C,CONTEXT> {
	using BASE = Reference<C,CONTEXT>;

public:
	using Handle = typename BASE::Handle;
	template<Const_Flag CC>	using Accessor = typename BASE::template Accessor<CC>;

	static constexpr bool Is_Const = C == CONST;

public:
	using BASE::BASE;
	//FORWARDING_CONSTRUCTOR(Accessor_Base, BASE) {}

	template<class VAL>
	auto& operator=(VAL&& val) { BASE::operator()() = std::forward<VAL>(val); return _self(); }

public:
	Accessor<C>& operator++() { ++BASE::data(); return _self(); }
	Accessor<C>& operator--() { --BASE::data(); return _self(); }

	// doesn't return accessor:
	auto operator++(int) { return BASE::operator()()++; }
	auto operator--(int) { return BASE::operator()()--; }

	template<class T> Accessor<C>& operator+=(T&& t) { BASE::data() += std::forward<T>(t); return _self(); }
	template<class T> Accessor<C>& operator-=(T&& t) { BASE::data() -= std::forward<T>(t); return _self(); }

// public:
// 	auto next() const { auto r = _self(); ++r.iterator(); return r; }
// 	auto prev() const { auto r = _self(); --r.iterator(); return r; }

	template<Const_Flag CC>
	bool operator==(const Accessor<CC>& o) const {
		static_assert(!has_member__Comparable<Accessor<C>>,
			"Accessors are not comparable by default - you need to turn on comparisons using Context::Accessor::Comparable");

		if constexpr(has_member__Comparable<Accessor<C>>) static_assert(Accessor<C>::Comparable,
			"Accessors are not comparable by default - you need to turn on comparisons using Context::Accessor::Comparable");
		
		return _self().handle() == o.handle();
	}

	template<Const_Flag CC>
	bool operator!=(const Accessor<CC>& o) const {
		return !operator==(o);
	}

private:
	using BASE::accessor; // turn off

public:
	CRTP_COMMON( Iterator_Base, Accessor<C> )
};













GENERATE_HAS_MEMBER(_get_comparable);
GENERATE_HAS_MEMBER(_will_compare_with);
GENERATE_HAS_MEMBER(_increment_n);
GENERATE_HAS_MEMBER(_decrement_n);

template<Const_Flag C, class CONTEXT>
class Iterator_Base : public Reference<C,CONTEXT> {
	using BASE = Reference<C,CONTEXT>;

public:
	using Handle = typename BASE::Handle;
	template<Const_Flag CC>	using Iterator = typename BASE::template Iterator<CC>;

public:
	using BASE::BASE;
	//FORWARDING_CONSTRUCTOR(Iterator_Base, BASE) {}


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
	auto&  operator++()&  { _self()._increment(); return            _self()  ; }
	auto&& operator++()&& { _self()._increment(); return std::move( _self() ); }

	auto&  operator--()&  { _self()._decrement(); return            _self()  ; }
	auto&& operator--()&& { _self()._decrement(); return std::move( _self() ); }


	auto operator++(int) { auto old = _self(); _self()._increment(); return old; }
	auto operator--(int) { auto old = _self(); _self()._decrement(); return old; }

public:
	auto next() const { auto r = _self(); ++r; return r; }
	auto prev() const { auto r = _self(); --r; return r; }

public:
	Iterator<C>& operator+=(int n) {
		if constexpr(has_member___increment_n< Iterator<C> >) _self()._increment_n(n);
		else for(int i=0; i<n; ++i) _self()._increment();
		return _self();
	}
	Iterator<C>& operator-=(int n) {
		if constexpr(has_member___decrement_n< Iterator<C> >) _self()._decrement_n(n);
		else for(int i=0; i<n; ++i) _self()._decrement();
		return _self();
	}


public:
	Iterator<C> operator+(int n) {
		auto iter = _self();
		iter += n;
		return iter;
	}

	Iterator<C> operator-(int n) {
		auto iter = _self();
		iter -= n;
		return iter;
	}


public:
	template<Const_Flag CC>
	bool operator==(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _get_comparable_base() == o._get_comparable_base();
	}

	template<Const_Flag CC>
	bool operator!=(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _get_comparable_base() != o._get_comparable_base();
	}

	template<Const_Flag CC>
	bool operator<(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _get_comparable_base() < o._get_comparable_base();
	}

	template<Const_Flag CC>
	bool operator>(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _get_comparable_base() > o._get_comparable_base();
	}

	template<Const_Flag CC>
	bool operator<=(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _get_comparable_base() <= o._get_comparable_base();
	}

	template<Const_Flag CC>
	bool operator>=(const Iterator<CC>& o) const {
		_will_compare_with_base(o);
		return _get_comparable_base() >= o._get_comparable_base();
	}


private:
	template<Const_Flag CC>
	void _will_compare_with_base(const Iterator<CC>& o) const {
		if constexpr(has_member___will_compare_with<BASE>) {
			BASE::_will_compare_with(o);
		}
		if constexpr(has_member___will_compare_with< Iterator<C> >) {
			_self()._will_compare_with(o);
		}
	}

	decltype(auto) _get_comparable_base() const {
		if constexpr(has_member___get_comparable< Iterator<C> >) {
			return _self()._get_comparable();
		}
		else {
			return BASE::handle();
		}
	}

public:
	// return accessor
	auto& operator*()       { return BASE::accessor(); }
	auto& operator*() const { return BASE::accessor(); }
	auto operator->()       { return &BASE::accessor(); }
	auto operator->() const { return &BASE::accessor(); }

private:
	using BASE::iterator; // turn off

public:
	CRTP_COMMON( Iterator_Base, Iterator<C> )
};






} // namespace internal
} // namespace salgo


