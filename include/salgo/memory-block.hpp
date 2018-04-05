#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "stack-storage.hpp"
#include "iterator-base.hpp"


namespace salgo {






namespace internal {
namespace Sparse_Vector {


template<bool> struct Add_num_existing { int num_existing = 0; };
template<> struct Add_num_existing<false> {};



template<class _VAL, bool _COUNTABLE>
struct Context {

	using Val = _VAL;
	static constexpr bool Countable = _COUNTABLE;

	//
	// forward declarations
	//

	struct Node;
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Sparse_Vector;







	struct Node {

		template<class... ARGS>
		Node(ARGS&&... args) {
			val.construct( std::forward<ARGS>(args)... );
		}

		~Node() {
			if(exists) {
				val.destruct();
			}
		}

		Stack_Storage<Val> val;
		bool exists = true;
	};






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		inline int key() const {
			return _key;
		}

		inline Const<Val,C>& val() {
			DCHECK( _owner.v[ _key ].exists ) << "accessing erased element";
			return _owner.at( _key );
		}

		inline const Val& val() const {
			DCHECK( _owner.v[ _key ].exists ) << "accessing erased element";
			return _owner.at( _key );
		}

		inline void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.erase( _key );
		}

		inline bool exists() const {
			return _owner.exists( _key );
		}


	private:
		Accessor(Const<Sparse_Vector,C>& owner, int key)
			: _owner(owner), _key(key) {}

		friend Sparse_Vector;
		friend Iterator<C>;


	private:
		Const<Sparse_Vector,C>& _owner;
		const int _key;
	};






	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Iterator> {

		// member functions accessed by BASE:
	private:
		friend Iterator_Base<C,Iterator>;

		inline void _increment() {
			do ++_key; while(_key != _owner.domain() && !_owner.v[ _key ].exists);
		}

		inline void _decrement() {
			do --_key; while(!_owner.v[ _key ].exists);
		}

		auto _get_comparable() const {  return _key;  }

		template<Const_Flag CC>
		auto _will_compare_with(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
		}



	public:
		inline auto operator*() const {  return Accessor<C>(_owner, _key);  }

		// unable to implement if using accessors:
		// auto operator->()       {  return &container[idx];  }




	private:
		inline Iterator(Const<Sparse_Vector,C>& owner, int key)
				: _owner(owner), _key(key) {
			if(key != owner.domain() && !owner.v[key].exists) _increment();
		}

		friend Sparse_Vector;

	private:
		Const<Sparse_Vector,C>& _owner;
		int _key;
	};








	class Sparse_Vector : private Add_num_existing<Countable> {
		using NUM_EXISTING_BASE = Add_num_existing<Countable>;

	public:
		using Val = Context::Val;
		static constexpr bool Countable = Context::Countable;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;

		friend Iterator<MUTAB>;
		friend Iterator<CONST>;


		//
		// data
		//
	private:
		std::vector<Node> v;


		//
		// construction
		//
	public:
		Sparse_Vector() = default;
		Sparse_Vector(int size) : v(size) {
			if constexpr(Countable) NUM_EXISTING_BASE::num_existing = size;
		}



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		inline void erase(int key) {
			_check_bounds(key);
			DCHECK( v[key].exists ) << "erasing already erased element";
			v[key].exists = false;
			v[key].val.destruct();
			if constexpr(Countable) --NUM_EXISTING_BASE::num_existing;
		}

		inline bool exists(int key) const {
			_check_bounds(key);
			return v[ key ].exists;
		}

		inline Val& operator[](int key) {
			_check_bounds(key);
			return v[key].val;
		}

		inline const Val& operator[](int key) const {
			_check_bounds(key);
			return v[key].val;
		}





	private:
		inline void _check_bounds(int key) const {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
		}





		//
		// interface
		//
	public:
		auto operator()(int key) {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
			return Accessor<MUTAB>(*this, key);
		}

		auto operator()(int key) const {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
			return Accessor<CONST>(*this, key);
		}

		template<class... ARGS>
		Accessor emplace_back(ARGS&&... args) {
			v.emplace_back( std::forward<ARGS>(args)... );
			if constexpr(Countable) ++NUM_EXISTING_BASE::num_existing;
			return Accessor<MUTAB>(*this, v.size()-1);
		}


		int count() const {
			static_assert(Countable, "count() called on non countable Sparse_Vector");
			return NUM_EXISTING_BASE::num_existing;
		}

		int domain() const {
			return v.size();
		}


		void reserve(int capacity) {
			v.reserve(capacity);
		}


		//
		// FUN is (int old_key, int new_key) -> void
		//
		template<class FUN>
		void compact(const FUN& fun = [](int,int){}) {
			int target = 0;
			for(int i=0; i<(int)v.size(); ++i) {
				if(v[i].exists && target != i) {
					v[target].val = std::move( v[i].val );
					v[target].exists = true;
					fun(i, target);
					++target;
				}
			}

			v.resize(target);
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







	struct With_Builder : Sparse_Vector {
		FORWARDING_CONSTRUCTOR(With_Builder, Sparse_Vector);

		using COUNTABLE =
			typename Context<Val,true> :: With_Builder;

		using FULL =
			typename Context<Val,true> :: With_Builder;
	};




}; // struct Context
} // namespace Sparse_Vector
} // namespace internal






template<
	class T
>
using Sparse_Vector = typename internal::Sparse_Vector::Context<
	T,
	false // COUNTABLE
> :: With_Builder;











}
