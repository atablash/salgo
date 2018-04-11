#pragma once

#include "const-flag.hpp"
#include "iterator-base.hpp"
#include "int-handle.hpp"
#include "vector.hpp"

#include <glog/logging.h>


namespace salgo {






namespace internal {
namespace Unordered_Vector {





template<class _VAL>
struct Context {

	using Val = _VAL;

	struct Handle : Int_Handle<Handle,int> {
		using BASE = Int_Handle<Handle,int>;
		FORWARDING_CONSTRUCTOR(Handle, BASE);
	};




	//
	// forward declarations
	//
	template<Const_Flag C> class Accessor;
	class Unordered_Vector;






	//
	// accessor / iterator
	//
	template<Const_Flag C>
	class Accessor : public Iterator_Base<C,Accessor> {
	public:
		auto     handle() const { DCHECK(!_just_erased); return _handle; }
		operator Handle() const { return handle(); }

		// get val
		auto& operator()()       { DCHECK(!_just_erased); return (*_owner)[ _handle ]; }
		auto& operator()() const { DCHECK(!_just_erased); return (*_owner)[ _handle ]; }
		operator auto&()       { return operator()(); }
		operator auto&() const { return operator()(); }


		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			DCHECK(!_just_erased);
			(*_owner)[_handle] = std::move( (*_owner).front() );
			(*_owner).pop_front();
			_just_erased = true;
		}


		// for ITERATOR_BASE:
	private:
		friend Iterator_Base<C,Accessor>;

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
		auto _will_compare_with(const Accessor<CC>& o) const {
			DCHECK_EQ(_owner, o._owner);
		}


	private:
		Accessor(Const<Unordered_Vector,C>* owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Unordered_Vector;


	private:
		Const<Unordered_Vector,C>* _owner;
		Handle _handle;

		bool _just_erased = false;
	};












	class Unordered_Vector {

	public:
		using Val = Context::Val;
		using Handle = Context::Handle;

		//
		// data
		//
	private:
		salgo::Vector<Val> v;


		//
		// construction
		//
	public:
		Unordered_Vector() = default;
		Unordered_Vector(int size) : v(size) {}



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

		void reserve(int capacity) { v.reserve(capacity); }



	public:
		auto begin()       { return _accessor( v.size()-1 ); }
		auto begin() const { return _accessor( v.size()-1 ); }

		auto end()       { return _accessor( -1 ); }
		auto end() const { return _accessor( -1 ); }


	private:
		auto _accessor(Handle handle)       { return Accessor<MUTAB>(this, handle); }
		auto _accessor(Handle handle) const { return Accessor<CONST>(this, handle); }

	private:
		void _check(Handle handle) const {
			DCHECK( 0 <= handle && handle < v.size() ) << "index " << handle << " out of bounds";
		}

	};











	//struct With_Builder : Unordered_Vector {
	//};




}; // struct Context
} // namespace Unordered_Vector
} // namespace internal






template<
	class T
>
using Unordered_Vector = typename internal::Unordered_Vector::Context<
	T
> :: Unordered_Vector;










}
