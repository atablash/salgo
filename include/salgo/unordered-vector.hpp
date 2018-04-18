#pragma once

#include "const-flag.hpp"
#include "accessors.hpp"
#include "int-handle.hpp"
#include "vector.hpp"

#include <glog/logging.h>


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




	struct Extra_Context {
		bool _just_erased = false;
	};


	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
	private:
		using BASE = Accessor_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Accessor, BASE) {}
		friend Unordered_Vector;

		using BASE::_container;
		using BASE::_handle;
		using BASE::_just_erased;

	public:
		int index() const { return BASE::handle(); }

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			DCHECK(!_just_erased);
			(*_container)[_handle].~Val();
			new(&(*_container)[_handle]) Val( std::move( (*_container).front()() ) );
			(*_container).pop_front();
			_just_erased = true;
		}
	};

	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
	private:
		using BASE = Iterator_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}
		friend Unordered_Vector;

		using BASE::_container;
		using BASE::_handle;
		using BASE::_just_erased;

	private:
		friend Iterator_Base<C,Context>;

		inline void _increment() {
			--_handle;
			_just_erased = false;
		}
		inline void _decrement() {
			if(!_just_erased) ++_handle;
			_just_erased = false;
		}

		auto _get_comparable() const { return _handle; }

		template<Const_Flag CC>
		auto _will_compare_with(const Iterator<CC>& o) const {
			DCHECK_EQ(_container, o._container);
		}
	};












	class Unordered_Vector {

	public:
		using Val = Context::Val;
		using Handle = Context::Handle;

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

		auto pop_front() {
			Val val = std::move( v[ v.size()-1 ] );
			v.pop_back();
			return val;
		}


		auto front()       { return _accessor( v.size()-1 ); }
		auto front() const { return _accessor( v.size()-1 ); }

		auto back()       { return _accessor( 0 ); }
		auto back() const { return _accessor( 0 ); }


		int size() const { return v.size(); }
		bool empty() const { return size() == 0; }

		void reserve(int capacity) { v.reserve(capacity); }



	public:
		auto begin()       { return _accessor( v.size()-1 ).iterator(); }
		auto begin() const { return _accessor( v.size()-1 ).iterator(); }

		auto end()       { return _accessor( -1 ).iterator(); }
		auto end() const { return _accessor( -1 ).iterator(); }


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
