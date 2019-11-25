#pragma once


namespace salgo {



namespace internal {
namespace reversed {


template<class Container>
struct Context {

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

	public:
		auto& iterator() { return _cast( BASE::iterator() ); }
	};




	template<Const_Flag C>
	class Iterator : public Base_Iterator<C> {
		using BASE = Base_Iterator<C>;

	public:
		auto&& operator++() && { return Context::_cast( std::move(*this).BASE::operator--() ); }
		auto&  operator++() &  { return Context::_cast( BASE::operator--() ); }

		auto&& operator--() && { return Context::_cast( std::move(*this).BASE::operator++() ); }
		auto&  operator--() &  { return Context::_cast( BASE::operator++() ); }

		auto operator++(int) { return _cast( BASE::operator--(0) ); }
		auto operator--(int) { return _cast( BASE::operator++(0) ); }


		auto&& operator+=(int i) && { return _cast( std::move(*this).BASE::operator-=(i) ); }
		auto&  operator+=(int i) &  { return _cast( BASE::operator-=(i) ); }

		auto&& operator-=(int i) && { return _cast( std::move(*this).BASE::operator+=(i) ); }
		auto&  operator-=(int i) &  { return _cast( BASE::operator+=(i) ); }


		auto operator+(int i) { return _cast( BASE::operator-(i) ); }
		auto operator-(int i) { return _cast( BASE::operator+(i) ); }

		auto& operator*() { return _cast( BASE::operator*() ); }
		auto operator->() { return &operator*(); }

	public:
		auto& accessor() { return _cast( BASE::accessor() ); }
	};







}; // struct Context



} // namespace reversed
} // namespace internal





template<class Container>
class Reversed {
	using Context = internal::reversed::Context<Container>;

public:
	Reversed(Container& cc) : c(cc) {}

	auto begin() {
		return ++Context::_cast( c.end() );
	}

	auto end() {
		return Context::_cast( c.before_begin() );
	}

private:
	Container& c;
};






} // namespace salgo


