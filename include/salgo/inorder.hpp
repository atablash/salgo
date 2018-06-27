#pragma once

#include "bst.hpp"


namespace salgo {


// forward
template<class Container>
class Inorder;



namespace internal {
namespace inorder {


template<class Container>
struct Context {


	using Handle = typename Container::Handle;


	template<Const_Flag C>
	using Base_Accessor = typename Container::template Accessor<C>;

	template<Const_Flag C>
	using Base_Iterator = typename Container::template Iterator<C>;


	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;



	//
	// static_cast to derived types, preserving cv-qualifiers
	//
	template<Const_Flag C> static auto& _cast(Base_Accessor<C>& x) { return *static_cast<Accessor<C>*>(&x); }
	template<Const_Flag C> static auto& _cast(Base_Iterator<C>& x) { return *static_cast<Iterator<C>*>(&x); }

	template<Const_Flag C> static auto&& _cast(Base_Accessor<C>&& x) { return std::move(*static_cast<Accessor<C>*>(&x)); }
	template<Const_Flag C> static auto&& _cast(Base_Iterator<C>&& x) { return std::move(*static_cast<Iterator<C>*>(&x)); }


	class Common_Data {
	protected:
		Handle cached_next;
		Handle cached_prev;
	};


	template<Const_Flag C>
	class Accessor : public Base_Accessor<C>, public Common_Data {
		using BASE = Base_Accessor<C>;

	private:
		friend Inorder<Container>;
		Accessor(BASE&& base_accessor) : BASE(base_accessor) { iterator()._init(); }

	public:		
		auto child(int ith)       { return Accessor<MUTAB>( std::move( BASE::child(ith) ) ); }
		auto child(int ith) const { return Accessor<CONST>( std::move( BASE::child(ith) ) ); }

		auto left()       { return Accessor<MUTAB>( std::move( BASE::left() ) ); }
		auto left() const { return Accessor<CONST>( std::move( BASE::left() ) ); }

		auto right()       { return Accessor<MUTAB>( std::move( BASE::right() ) ); }
		auto right() const { return Accessor<CONST>( std::move( BASE::right() ) ); }

		auto parent()       { return Accessor<MUTAB>( std::move( BASE::parent() ) ); }
		auto parent() const { return Accessor<CONST>( std::move( BASE::parent() ) ); }


	public:
		auto& iterator() { return _cast( BASE::iterator() ); }
	};




	template<Const_Flag C>
	class Iterator : public Base_Iterator<C>, public Common_Data {
		using BASE = Base_Iterator<C>;

		using Common_Data::cached_prev;
		using Common_Data::cached_next;

	private:
		friend Inorder<Container>;
		Iterator(BASE&& base_iterator) : BASE(base_iterator) { _init(); }

	public:
		auto&& operator++() && { increment(); return std::move(*this); }
		auto&  operator++() &  { increment(); return *this; }

		auto&& operator--() && { decrement(); return std::move(*this); }
		auto&  operator--() &  { decrement(); return *this; }

		auto operator++(int) { auto r = *this; increment(); return r; }
		auto operator--(int) { auto r = *this; decrement(); return r; }

	private:
		void increment() {
			BASE::_handle() = cached_next;
			update_next();
		}

		void decrement() {
			BASE::_handle() = cached_prev;
			update_prev();
		}

		void update_next() {
			cached_next = bst_next( BASE::accessor() );
		}

		void update_prev() {
			cached_prev = bst_prev( BASE::accessor() );
		}

	private:
		friend Accessor<C>;
		void _init() {
			if(!BASE::_handle().valid()) return; // does not exist
			update_prev();
			update_next();
		}


	private:
		using BASE::operator+=;
		using BASE::operator-=;
		using BASE::operator+;
		using BASE::operator-;

	public:
		auto& operator*() { return _cast( BASE::operator*() ); }
		auto operator->() { return &operator*(); }

	public:
		auto& accessor() { return _cast( BASE::accessor() ); }
	};







}; // struct Context



} // namespace reversed
} // namespace internal





template<class Container>
class Inorder {
	using Context = internal::inorder::Context<Container>;

	template<Const_Flag C>
	using Iterator = typename Context::template Iterator<C>;

	template<Const_Flag C>
	using Base_Iterator = typename Context::template Base_Iterator<C>;

public:
	Inorder(Container& cc) : c(cc) {}

	auto before_begin() {
		return create_iterator( c.before_begin() );
	}

	auto begin() {
		return create_iterator( c.begin() );
	}

	auto end() {
		return create_iterator( c.end() );
	}

private:
	auto create_iterator( Iterator<MUTAB>&& base_iterator ) {
		return Iterator<MUTAB>( std::move(base_iterator) );
	}

private:
	Container& c;
};






} // namespace salgo


