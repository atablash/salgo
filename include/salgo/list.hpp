#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "iterator-base.hpp"


namespace salgo {






namespace internal {
namespace List {





template<bool> struct Add_num_existing { int num_existing = 0; };
template<> struct Add_num_existing<false> {};





template<class _VAL, class _STORAGE, bool _COUNTABLE>
struct Context {

	//
	// forward declarations
	//
	struct Node;
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class List;


	//
	// template arguments
	//
	using Val = _VAL;
	using Storage = _STORAGE :: VAL<Node>;
	static constexpr bool Countable = _COUNTABLE;

	using Handle = Storage::Handle;








	struct Node {
		Val val;

		Handle next = Handle();
		Handle prev = Handle();

		template<class... ARGS>
		Node(ARGS&&... args) : val( std::forward<ARGS>(args)... ) {}
	};






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		inline int handle() const {
			return _handle;
		}

		inline Const<Val,C>& val() {
			return _owner.at( _handle ).val;
		}

		inline const Val& val() const {
			return _owner.at( _handle ).val;
		}

		inline void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.erase( _handle );
		}

		inline bool exists() const {
			return _owner.exists( _handle );
		}


	private:
		Accessor(Const<List,C>& owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend List;
		friend Iterator<C>;


	private:
		Const<List,C>& _owner;
		const Handle _handle;
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
			_handle = _owner.at( _handle ).next;
			DCHECK( _handle.valid() ) << "followed broken list link";
		}

		inline void _decrement() {
			_handle = _owner.at( _handle ).prev;
			DCHECK( _handle.valid() ) << "followed broken list link";
		}

		auto _get_comparable() const {  return _handle;  }

		template<Const_Flag CC>
		auto _will_compare_with(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
		}



	public:
		inline auto operator*() const {  return Accessor<C>(_owner, _handle);  }

		// unable to implement if using accessors:
		// auto operator->()       {  return &container[idx];  }




	private:
		inline Iterator(Const<List,C>& owner, Handle handle)
				: _owner(owner), _handle(handle) {
			if(key != owner.domain() && !owner.v[key].exists) _increment();
		}

		friend List;

	private:
		Const<List,C>& _owner;
		Handle _handle;
	};








	class List : private Add_num_existing<Countable> {
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
		Storage v;


		//
		// construction
		//
	public:
		List() = default;
		List(int size) {
			if constexpr(Countable) NUM_EXISTING_BASE::num_existing = size;

			Handle prev = Handle();

			for(int i=0; i<size; ++i) {
				auto handle = v.construct().handle();
				v[handle].prev = prev;
				if(prev) v[prev].next = handle;
				prev = handle;
			}
		}



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		inline void erase(Handle handle) {
			DCHECK( v[handle].exists ) << "erasing already erased element";
			v[key].exists = false;
			v[key].val.destruct();
			if constexpr(Countable) --NUM_EXISTING_BASE::num_existing;
		}

		inline bool exists(int key) const {
			_check_bounds(key);
			return v[ key ].exists;
		}

		inline Val& at(int key) {
			_check_bounds(key);
			return v[key].val;
		}

		inline const Val& at(int key) const {
			_check_bounds(key);
			return v[key].val;
		}





		//
		// interface
		//
	public:
		auto operator[](int key) {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
			return Accessor<MUTAB>(*this, key);
		}

		auto operator[](int key) const {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
			return Accessor<CONST>(*this, key);
		}

		template<class... ARGS>
		void emplace_back(ARGS&&... args) {
			v.emplace_back( std::forward<ARGS>(args)... );
			if constexpr(Countable) ++NUM_EXISTING_BASE::num_existing;
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
