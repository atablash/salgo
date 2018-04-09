#pragma once

#include "const-flag.hpp"
#include "iterator-base.hpp"

#include <glog/logging.h>


namespace salgo {






namespace internal {
namespace Unordered_Vector {



template<class _VAL>
struct Context {

	using Val = _VAL;

	//
	// forward declarations
	//
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Unordered_Vector;



	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		inline int handle() const {
			return _key;
		}

		inline Const<Val,C>& val() {
			return _owner[ _key ];
		}

		inline const Val& val() const {
			return _owner[ _key ];
		}

		inline void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.erase( _key );
		}


	private:
		Accessor(Const<Unordered_Vector,C>& owner, int key)
			: _owner(owner), _key(key) {}

		friend Unordered_Vector;
		friend Iterator<C>;


	private:
		Const<Unordered_Vector,C>& _owner;
		const int _key;
	};





	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Iterator>{


			// member functions accessed by BASE:
		private:
			friend Iterator_Base<C, Iterator>;

			inline void _increment() {
				++_key;
			}

			inline void _decrement() {
				--_key;
			}

			auto _get_comparable() const {
				return _key;
			}

			template<Const_Flag CC>
			auto _will_compare_with(const Iterator<CC>& o) const {
				DCHECK_EQ(&_owner, &o._owner);
			}



	public:
		inline auto operator*() const {  return Accessor<C>(_owner, _key);  }

		// unable to implement if using accessors:
		// auto operator->()       {  return &container[idx];  }



	private:
		inline Iterator(Const<Unordered_Vector,C>& owner, int key)
				: _owner(owner), _key(key) {}

		friend Unordered_Vector;

	private:
		Const<Unordered_Vector,C>& _owner;
		int _key;
	};
















	class Unordered_Vector {

	public:
		using Val = Context::Val;

		using Handle = int;

		//
		// data
		//
	private:
		std::vector<Val> v;


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
		inline void erase(Handle handle) {
			_check_bounds(handle);
			v[handle] = std::move( v.back() );
			v.pop_back();
		}

		inline Val& operator[](Handle handle) {
			_check_bounds(handle);
			return v[handle];
		}

		inline const Val& operator[](Handle handle) const {
			_check_bounds(handle);
			return v[handle];
		}



	private:
		inline void _check_bounds(Handle handle) const {
			DCHECK_GE( handle, 0 ) << "index out of bounds";
			DCHECK_LT( handle, (int)v.size() ) << "index out of bounds";
		}






		//
		// interface
		//
	public:
		auto operator()(int handle) {
			DCHECK_GE( handle, 0 ) << "index out of bounds";
			DCHECK_LT( handle, (int)v.size() ) << "index out of bounds";
			return Accessor<MUTAB>(*this, handle);
		}

		auto operator()(int handle) const {
			DCHECK_GE( handle, 0 ) << "index out of bounds";
			DCHECK_LT( handle, (int)v.size() ) << "index out of bounds";
			return Accessor<CONST>(*this, handle);
		}

		template<class... ARGS>
		void add(ARGS&&... args) {
			v.emplace_back( std::forward<ARGS>(args)... );
		}


		int size() const {
			return v.size();
		}


		void reserve(int capacity) {
			v.reserve(capacity);
		}



	public:
		inline auto begin() {
			return Iterator<MUTAB>(*this, 0);
		}

		inline auto begin() const {
			return Iterator<CONST>(*this, 0);
		}

		inline auto cbegin() const {
			return Iterator<CONST>(*this, 0);
		}


		inline auto end() {
			return Iterator<MUTAB>(*this, v.size());
		}

		inline auto end() const {
			return Iterator<CONST>(*this, v.size());
		}

		inline auto cend() const {
			return Iterator<CONST>(*this, v.size());
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
