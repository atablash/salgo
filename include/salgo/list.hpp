#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "iterator-base.hpp"
#include "stack-storage.hpp"
#include "allocator.hpp"


namespace salgo {






namespace internal {
namespace List {





template<bool> struct Add_num_existing { int num_existing = 0; };
template<> struct Add_num_existing<false> {};





template<class _VAL, class _ALLOCATOR, bool _COUNTABLE>
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
	using Allocator = typename _ALLOCATOR :: template VAL<Node>;
	static constexpr bool Countable = _COUNTABLE;

	using Handle = typename Allocator :: Small_Handle; // Small_Handle is faster than Handle








	struct Node {
		typename salgo::Stack_Storage<Val>::PERSISTENT val; // make sure it's not moved

		Handle next = Handle();
		Handle prev = Handle();
	};






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		auto handle() const {
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
			_handle = _owner->_alloc()[ _handle ].next;
			DCHECK( _handle.valid() ) << "followed broken list link";
		}

		inline void _decrement() {
			_handle = _owner->_alloc()[ _handle ].prev;
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








	class List :
			private Allocator,
			private Add_num_existing<Countable> {

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
		Handle _front;
		Handle _back;

	private:
		auto& _alloc()       { return *static_cast<Allocator*>(this); }
		auto& _alloc() const { return *static_cast<Allocator*>(this); }

		//
		// construction
		//
	public:
		List() {
			_front = _alloc().construct().handle();
			_back = _alloc().construct().handle();
			_alloc()[_front].next = _back;
			_alloc()[_back].prev = _front;
			#ifndef NDEBUG
			_alloc()[_front].prev.reset();
			_alloc()[_back].next.reset();
			#endif
		};

		List(int size) : List() {
			if constexpr(Countable) NUM_EXISTING_BASE::num_existing = size;

			for(int i=0; i<size; ++i) {
				emplace( _back );
			}
		}

		~List() {
			static_assert(std::is_trivially_destructible_v<Node>);

			for(auto e : *this) {
				_alloc()[ e.handle() ].val.destruct();
				//_alloc().destruct( e.handle() );
			}

			//_alloc().destruct( _front );
			//_alloc().destruct( _back );
		}



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		void erase(Handle handle) {
			auto next = _alloc()[handle].next;
			auto prev = _alloc()[handle].prev;

			DCHECK(prev.valid());
			DCHECK(next.valid());

			_alloc()[prev].next = next;
			_alloc()[next].prev = prev;

			_alloc()[handle].val.destruct();
			_alloc().destruct(handle);

			if constexpr(Countable) --NUM_EXISTING_BASE::num_existing;
		}

		//bool exists(Handle handle) const {
		//	return v[ key ].exists;
		//}

		Val& operator[](Handle handle) {
			return _alloc()[handle].val;
		}

		const Val& operator[](Handle handle) const {
			return _alloc()[handle].val;
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
			DCHECK( where.valid() );

			auto h = _alloc().construct().handle();
			_alloc()[h].val.construct( std::forward<ARGS>(args)... );

			DCHECK( _alloc()[where].prev.valid() );
			_alloc()[h].prev = _alloc()[where].prev;
			_alloc()[h].next = where;
			_alloc()[_alloc()[h].prev].next = h;
			_alloc()[_alloc()[h].next].prev = h;
			if constexpr(Countable) ++NUM_EXISTING_BASE::num_existing;

			return Accessor<MUTAB>(*this, h);
		}

		template<class... ARGS>
		auto emplace_front(ARGS&&... args) {
			return emplace( _alloc()[_front].next, std::forward<ARGS>(args)... );
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
			return Iterator<MUTAB>(this, _alloc()[_front].next);
		}

		inline auto begin() const {
			return Iterator<CONST>(this, _alloc()[_front].next);
		}

		inline auto cbegin() const {
			return Iterator<CONST>(this, _alloc()[_front].next);
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

		template<class NEW_ALLOCATOR>
		using ALLOCATOR =
			typename Context<Val, NEW_ALLOCATOR, Countable> :: With_Builder;

		using COUNTABLE =
			typename Context<Val, Allocator, true> :: With_Builder;

		using FULL =
			typename Context<Val, Allocator, true> :: With_Builder;
	};




}; // struct Context
} // namespace Sparse_Vector
} // namespace internal






template<
	class VAL
>
using List = typename internal::List::Context<
	VAL,
	Allocator<VAL>,
	false // COUNTABLE
> :: With_Builder;











}
