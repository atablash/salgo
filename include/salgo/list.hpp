#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "iterator-base.hpp"
#include "stack-storage.hpp"
#include "sparse-vector-storage.hpp"


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
	using Storage = typename _STORAGE :: template VAL<Node>;
	static constexpr bool Countable = _COUNTABLE;

	using Handle = typename Storage::Handle;








	struct Node {
		Stack_Storage<Val> val;

		Handle next = Handle();
		Handle prev = Handle();
	};






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		int handle() const {
			return _handle;
		}

		Const<Val,C>& val() {
			return _owner[ _handle ];
		}

		const Val& val() const {
			return _owner[ _handle ];
		}

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.erase( _handle );
		}

		//bool exists() const {
		//	return _owner.exists( _handle );
		//}


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
			_handle = _owner->v[ _handle ].next;
			DCHECK( _handle.valid() ) << "followed broken list link";
		}

		inline void _decrement() {
			_handle = _owner->v[ _handle ].prev;
			DCHECK( _handle.valid() ) << "followed broken list link";
		}

		auto _get_comparable() const {  return _handle;  }

		template<Const_Flag CC>
		auto _will_compare_with(const Iterator<CC>& o) const {
			DCHECK_EQ(_owner, o._owner);
		}



	public:
		inline auto operator*() const {  return Accessor<C>(*_owner, _handle);  }

		// unable to implement if using accessors:
		// auto operator->()       {  return &container[idx];  }




	private:
		inline Iterator(Const<List,C>* owner, Handle handle) : _owner(owner), _handle(handle) {}
		friend List;

	private:
		Const<List,C>* _owner;
		Handle _handle;
	};








	class List : private Add_num_existing<Countable> {
		using NUM_EXISTING_BASE = Add_num_existing<Countable>;

	public:
		using Val = Context::Val;
		static constexpr bool Is_Countable = Context::Countable;


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
		Handle _front;
		Handle _back;


		//
		// construction
		//
	public:
		List() {
			_front = v.construct().handle();
			_back = v.construct().handle();
			v[_front].next = _back;
			v[_back].prev = _front;
			#ifndef NDEBUG
			v[_front].prev.reset();
			v[_back].next.reset();
			#endif
		};

		List(int size) : List() {
			if constexpr(Countable) NUM_EXISTING_BASE::num_existing = size;

			for(int i=0; i<size; ++i) {
				emplace( _back );
			}
		}

		~List() {
			for(auto e : *this) {
				v[ e.handle() ].val.destruct();
				v.destruct( e.handle() );
			}

			v.destruct( _front );
			v.destruct( _back );
		}



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		void erase(Handle handle) {
			auto next = v[handle].next;
			auto prev = v[handle].prev;

			DCHECK(prev);
			DCHECK(next);

			v[prev].next = next;
			v[next].prev = prev;

			v[handle].val.destruct();
			v.destruct(handle);

			if constexpr(Countable) --NUM_EXISTING_BASE::num_existing;
		}

		//bool exists(Handle handle) const {
		//	return v[ key ].exists;
		//}

		Val& operator[](Handle handle) {
			return v[handle].val;
		}

		const Val& operator[](Handle handle) const {
			return v[handle].val;
		}





		//
		// interface
		//
	public:
		auto operator()(Handle handle) {
			return Accessor<MUTAB>(*this, handle);
		}

		auto operator()(Handle handle) const {
			return Accessor<CONST>(*this, handle);
		}

		template<class... ARGS>
		auto emplace(Handle where, ARGS&&... args) {
			DCHECK(where);

			auto h = v.construct().handle();
			v[h].val.construct( std::forward<ARGS>(args)... );

			CHECK( v[h].prev );
			v[h].prev = v[where].prev;
			v[h].next = where;
			v[v[h].prev].next = h;
			v[v[h].next].prev = h;
			if constexpr(Countable) ++NUM_EXISTING_BASE::num_existing;

			return Accessor<MUTAB>(*this, h);
		}

		template<class... ARGS>
		auto emplace_front(ARGS&&... args) {
			return emplace( v[_front].next, std::forward<ARGS>(args)... );
		}

		template<class... ARGS>
		auto emplace_back(ARGS&&... args) {
			return emplace( _back, std::forward<ARGS>(args)... );
		}


		int count() const {
			static_assert(Countable, "count() called on non countable Sparse_Vector");
			return NUM_EXISTING_BASE::num_existing;
		}





	public:
		inline auto begin() {
			return Iterator<MUTAB>(this, v[_front].next);
		}

		inline auto begin() const {
			return Iterator<CONST>(this, v[_front].next);
		}

		inline auto cbegin() const {
			return Iterator<CONST>(this, v[_front].next);
		}


		inline auto end() {
			return Iterator<MUTAB>(this, _back);
		}

		inline auto end() const {
			return Iterator<CONST>(this, _back);
		}

		inline auto cend() const {
			return Iterator<CONST>(this, _back);
		}

	};







	struct With_Builder : List {
		FORWARDING_CONSTRUCTOR(With_Builder, List);

		template<class NEW_STORAGE>
		using STORAGE =
			typename Context<Val, NEW_STORAGE, Countable> :: With_Builder;

		using COUNTABLE =
			typename Context<Val, Storage, true> :: With_Builder;

		using FULL =
			typename Context<Val, Storage, true> :: With_Builder;
	};




}; // struct Context
} // namespace Sparse_Vector
} // namespace internal






template<
	class VAL
>
using List = typename internal::List::Context<
	VAL,
	Sparse_Vector_Storage<VAL>,
	false // COUNTABLE
> :: With_Builder;











}
