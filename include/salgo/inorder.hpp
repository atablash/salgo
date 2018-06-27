#pragma once

#include "bst.hpp"



#include "helper-macros-on"
namespace salgo {





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




	template<Const_Flag C>
	class Accessor : public Base_Accessor<C> {
		using BASE = Base_Accessor<C>;

	private:
		Accessor(BASE&& base_accessor) : BASE(base_accessor) {}

	public:		
		auto child(int ith)       { return _cast( BASE::child(ith) ); }
		auto child(int ith) const { return _cast( BASE::child(ith) ); }

		auto left()       { return _cast( BASE::left() ); }
		auto left() const { return _cast( BASE::left() ); }

		auto right()       { return _cast( BASE::right() ); }
		auto right() const { return _cast( BASE::right() ); }

		auto parent()       { return _cast( BASE::parent() ); }
		auto parent() const { return _cast( BASE::parent() ); }


	public:
		auto& iterator() { return _cast( BASE::iterator() ); }
	};





	class End_Iterator {};




	template<Const_Flag C>
	class Iterator : public Base_Iterator<C> {
		using BASE = Base_Iterator<C>;

	private:
		Iterator(BASE&& base_iterator) : BASE(base_iterator) {}

	public:
		auto&& operator++() && { increment(); return std::move(*this); }
		auto&  operator++() &  { increment(); return *this; }

		auto&& operator--() && { decrement(); return std::move(*this); }
		auto&  operator--() &  { decrement(); return *this; }

		auto operator++(int) { auto r = *this; increment(); return r; }
		auto operator--(int) { auto r = *this; decrement(); return r; }

	private:
		void increment() {
			HA = bst_next( accessor() );
		}

		void decrement() {
			HA = bst_prev( accessor() );
		}

	private:
		friend Accessor<C>;

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

	public:
		bool operator!=(End_Iterator) const { return HA.valid(); }
	};




	class Inorder {

	public:
		Inorder(const typename Container::template Accessor<MUTAB>& v) : root(v) { DCHECK( !root.parent() ); }

		auto begin() {
			auto first = root;
			while( first.left() ) first = first.left();
			return _cast( std::move(first).iterator() );
		}

		auto end() { return End_Iterator(); }

	private:
		typename Container::template Accessor<MUTAB> root;
	};



}; // struct Context



} // namespace reversed
} // namespace internal








template<class V>
auto Inorder(const V& root) {
	return typename internal::inorder::Context< typename V::Container >::Inorder(root);
}







} // namespace salgo
#include "helper-macros-off"


