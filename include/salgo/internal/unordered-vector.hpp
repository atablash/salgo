#pragma once

#include "const-flag.hpp"
#include "accessors.hpp"
#include "handles.hpp"
#include "vector.hpp"

#include <glog/logging.h>

#include "helper-macros-on"

namespace salgo {






namespace internal {
namespace unordered_vector {





template<class _VAL, class _VECTOR>
struct Context {

	using Val = _VAL;
	using Vector = _VECTOR;

	struct Handle : Int_Handle_Base<Handle,int> {
		using BASE = Int_Handle_Base<Handle,int>;
		FORWARDING_CONSTRUCTOR(Handle, BASE) {}
	};




	//
	// forward declarations
	//
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Unordered_Vector;
	using Container = Unordered_Vector;





	//
	// HACK for supporting backward range-based-for iteration with erasing elements
	//
	template<Const_Flag C>
	struct End_Iterator : Iterator<C> {
		using BASE = Iterator<C>;
		FORWARDING_CONSTRUCTOR(End_Iterator, BASE) {}


		// forbid decrementing
	private:
		using BASE::operator--;
		using BASE::operator-=;
		using BASE::operator++;
		using BASE::operator+=;
	};







	template<Const_Flag C>
	class Reference : public Reference_Base<C,Context> {
		using BASE = Reference_Base<C,Context>;
		
	public:
		FORWARDING_CONSTRUCTOR(Reference, BASE) {}

	protected:
		bool _just_erased = false;
	};


	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		friend Unordered_Vector;

		using BASE::_just_erased;

	public:
		using BASE::operator=;

	public:
		int index() const { return BASE::handle(); }

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			DCHECK(!_just_erased);
			CONT[HANDLE].~Val();
			new(&CONT[HANDLE]) Val( std::move( CONT.back()()) );
			CONT.pop_back();
			_just_erased = true;
		}
	};

	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
		friend Unordered_Vector;
		friend End_Iterator<C>;

		using BASE::_just_erased;

	private:
		friend Iterator_Base<C,Context>;

		inline void _increment() {
			if(!_just_erased) ++HANDLE;
			_just_erased = false;
		}
		inline void _decrement() {
			--HANDLE;
			_just_erased = false;
		}

		//
		// compare with End_Iterator
		//
	public:
		template<Const_Flag CC>
		bool operator!=(const End_Iterator<CC>&) { return BASE::operator!=( CONT.end() ); }
		using BASE::operator!=; // still use normal version too
	};












	class Unordered_Vector {

	public:
		using Val = Context::Val;
		using Handle = Context::Handle;

		template<Const_Flag C> using Accessor = Accessor<C>;
		template<Const_Flag C> using Iterator = Iterator<C>;

		//
		// data
		//
	private:
		Vector v;


		//
		// construction
		//
	public:
		Unordered_Vector() = default;
		Unordered_Vector(int size) : v(size) {}

		template<class T>
		Unordered_Vector(std::initializer_list<T>&& il) : v(std::move(il)) {}


		Unordered_Vector(const Unordered_Vector&) = default;
		Unordered_Vector(Unordered_Vector&&) = default;

		Unordered_Vector& operator=(const Unordered_Vector&) = default;
		Unordered_Vector& operator=(Unordered_Vector&&) = default;



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		auto& operator[](Handle handle)       { _check(handle); return v[handle]; }
		auto& operator[](Handle handle) const { _check(handle); return v[handle]; }

		// inline void erase(Handle handle) = delete; // only through accessor




		//
		// interface
		//
	public:
		auto operator()(Handle handle)       { _check(handle); return _accessor(handle); }
		auto operator()(Handle handle) const { _check(handle); return _accessor(handle); }

		template<class... ARGS>
		auto add(ARGS&&... args) {
			v.emplace_back( std::forward<ARGS>(args)... );
			return _accessor( v.size()-1 );
		}

		auto pop_back() {
			Val val = std::move( v[ v.size()-1 ] );
			v.pop_back();
			return val;
		}

		auto front()       { return _accessor( 0 ); }
		auto front() const { return _accessor( 0 ); }

		auto back()       { return _accessor( v.size()-1 ); }
		auto back() const { return _accessor( v.size()-1 ); }


		int size() const { return v.size(); }
		bool empty() const { return size() == 0; }

		void reserve(int capacity) { v.reserve(capacity); }



	public:
		auto before_begin()       { return _accessor( -1 ).iterator(); }
		auto before_begin() const { return _accessor( -1 ).iterator(); }

		auto begin()       { return _accessor( 0 ).iterator(); }
		auto begin() const { return _accessor( 0 ).iterator(); }

		auto end()       { return End_Iterator<MUTAB>( this, v.size() ); }
		auto end() const { return End_Iterator<CONST>( this, v.size() ); }


	private:
		auto _accessor(Handle handle)       { return Accessor<MUTAB>(this, handle); }
		auto _accessor(Handle handle) const { return Accessor<CONST>(this, handle); }

	private:
		void _check(Handle handle) const {
			DCHECK( 0 <= handle && handle < v.size() ) << "index " << handle << " out of bounds";
		}

	};











	struct With_Builder : Unordered_Vector {
		FORWARDING_CONSTRUCTOR(With_Builder, Unordered_Vector) {}
		FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(With_Builder, Unordered_Vector) {}

		template<int X>
		using INPLACE_BUFFER = typename Context<Val, typename Vector :: template INPLACE_BUFFER<X>> :: With_Builder;
	};




}; // struct Context
} // namespace Unordered_Vector
} // namespace internal






template<
	class T
>
using Unordered_Vector = typename internal::unordered_vector::Context<
	T,
	salgo::Vector<T> // VECTOR
> :: With_Builder;










}

#include "helper-macros-off"
