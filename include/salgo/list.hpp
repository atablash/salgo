#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "accessors.hpp"
#include "stack-storage.hpp"
//#include "crude-allocator.hpp"
//#include "random-allocator.hpp"
#include "vector-allocator.hpp"

#ifndef NDEBUG
#include <unordered_set>
#endif


namespace salgo {





//
// operations on list nodes
//
template<class ALLOC, class HANDLE>
inline auto list_next(const ALLOC& alloc, HANDLE handle) { DCHECK(handle.valid()); return alloc[handle].next; }

template<class ALLOC, class HANDLE>
inline auto list_prev(const ALLOC& alloc, HANDLE handle) { DCHECK(handle.valid()); return alloc[handle].prev; }



namespace internal {
	template<class ALLOC, class HANDLE>
	inline auto list_erase(ALLOC& alloc, HANDLE handle) {
		auto& me = alloc[handle];

		auto prev = me.prev;
		auto next = me.next;

		DCHECK( prev.valid() ) << "erasing dummy list element";
		DCHECK( next.valid() ) << "erasing dummy list element";

		alloc[prev].next = next;
		alloc[next].prev = prev;

		me.val.destruct();
		alloc.destruct(handle);

		return next;
	}
}


template<class ALLOC, class HANDLE>
inline auto list_erase(ALLOC& alloc, HANDLE handle) {

	static_assert(! decltype(alloc[handle])::Countable,
		"element of COUNTABLE list must be erased though the list object");

	return internal::list_erase(alloc, handle);
}







namespace internal {
	template<class ALLOC, class HANDLE, class... ARGS>
	inline auto list_emplace(ALLOC& alloc, HANDLE where, ARGS&&... args) {
		DCHECK( where.valid() ) << "handle invalid";

		auto h = alloc.construct_near(where).handle();
		alloc[h].val.construct( std::forward<ARGS>(args)... );

		DCHECK( alloc[where].prev.valid() ) << "where->prev link invalid";
		alloc[h].prev = alloc[where].prev;
		alloc[h].next = where;
		alloc[alloc[h].prev].next = h;
		alloc[alloc[h].next].prev = h;

		return h;
	}
}


template<class ALLOC, class HANDLE, class... ARGS>
inline auto list_emplace(ALLOC& alloc, HANDLE where, ARGS&&... args) {

	static_assert(! decltype(alloc[where])::Countable,
		"element of COUNTABLE list must be emplaced though the list object");

	return internal::list_emplace(alloc, where, std::forward<ARGS>(args)...);
}











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
	using Container = List;




	//
	// template arguments
	//
	using Val = _VAL;
	static constexpr bool Countable = _COUNTABLE;






	using Allocator = typename _ALLOCATOR :: template VAL<Node>;

	using       Handle = typename Allocator ::       Handle;
	using Small_Handle = typename Allocator :: Small_Handle;









	template<Const_Flag C>
	class Reference : public Reference_Base<C,Context> {
		using BASE = Reference_Base<C,Context>;

	protected:
		using BASE::_container;
		using BASE::_handle;

	public:
		FORWARDING_CONSTRUCTOR(Reference, BASE) {}

	private:
		Small_Handle _prev;
		Small_Handle _next;
		bool just_erased = false;

	protected:
		void on_erase() {
			_next = list_next(_container()._alloc(), _handle() );
			_prev = list_prev(_container()._alloc(), _handle() );
			just_erased = true;
		}

		void reset() {
			_prev.reset();
			_next.reset();
			just_erased = false;
		}

		auto get_next() {
			if(just_erased) return _next;
			else return list_next(_container()._alloc(), _handle() );
		}

		auto get_prev() {
			if(just_erased) return _prev;
			else return list_prev(_container()._alloc(), _handle() );
		}
	};


	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

	public:
		FORWARDING_CONSTRUCTOR(Accessor,BASE) {}

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			BASE::on_erase();
			_container().erase( _handle() );
		}

		auto next()       { return _container()( BASE::get_next() ); }
		auto next() const { return _container()( BASE::get_next() ); }

		auto prev()       { return _container()( BASE::get_prev() ); }
		auto prev() const { return _container()( BASE::get_prev() ); }
	};




	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		using BASE::_container;
		using BASE::_handle;

	public:
		FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() {
			_handle() = BASE::get_next();
			BASE::reset();
			DCHECK( _handle().valid() ) << "followed broken list link";
		}

		void _decrement() {
			_handle() = BASE::get_prev();
			BASE::reset();
			DCHECK( _handle().valid() ) << "followed broken list link";
		}
	};















	struct Node {
		// typename salgo::Stack_Storage<Val>::PERSISTENT val; // make sure it's not moved
		typename salgo::Stack_Storage<Val> val; // val can be moved

		Small_Handle next;
		Small_Handle prev;

		static constexpr bool Countable = Context::Countable;
	};








	class List :
			private Allocator,
			private Add_num_existing<Countable> {

		using NUM_EXISTING_BASE = Add_num_existing<Countable>;

	public:
		using Val = Context::Val;
		static constexpr bool Is_Countable = Context::Countable;

		using       Handle = Context::      Handle;
		using Small_Handle = Context::Small_Handle;


	private:
		//friend Accessor<MUTAB>;
		//friend Accessor<CONST>;
		//friend Iterator<MUTAB>;
		//friend Iterator<CONST>;
		friend Reference<MUTAB>;
		friend Reference<CONST>;


		//
		// data
		//
	private:
		Handle _front;
		Handle _back;

	private:
		auto& _alloc()       { return *static_cast<      Allocator*>(this); }
		auto& _alloc() const { return *static_cast<const Allocator*>(this); }

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
			for(int i=0; i<size; ++i) {
				emplace( _back );
			}
		}

		~List() {
			for(auto& e : *this) {
				_alloc()[ e.handle() ].val.destruct();
				_alloc().destruct( e.handle() );
			}

			_alloc().destruct( _front );
			_alloc().destruct( _back );
		}

		void clear() {
			#ifdef NDEBUG
			static_assert(std::is_trivially_destructible_v<Node>);
			#endif

			for(auto e : *this) {
				_alloc()[ e.handle() ].val.destruct();
				_alloc().destruct( e.handle() );
			}

			_alloc()[_front].next = _back;
			_alloc()[_back].prev = _front;
		}




	public:
		// copy
		List(const List& o) : List() {
			*this = o;
		}

		// trivial move
		List(List&& o) = default;

		// copy assignment
		List& operator=(const List& o) {
			clear();
			*this += o;
		}

		// move assignment: todo

		List& operator+=(const List& o) {
			for(auto e : o) {
				emplace(_back, e());
			}
		}

		// todo
		//List& operator+=(List&& o) {}



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		void erase(Handle handle) {
			internal::list_erase( _alloc(), handle );
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
			return Accessor<MUTAB>(this, handle);
		}

		auto operator()(Handle handle) const {
			return Accessor<CONST>(this, handle);
		}

		auto next(Handle handle)       { return Accessor<MUTAB>(this, list_next(_alloc(), handle)); }
		auto next(Handle handle) const { return Accessor<CONST>(this, list_next(_alloc(), handle)); }

		auto prev(Handle handle)       { return Accessor<MUTAB>(this, list_prev(_alloc(), handle)); }
		auto prev(Handle handle) const { return Accessor<CONST>(this, list_prev(_alloc(), handle)); }


		template<class... ARGS>
		auto emplace(Handle where, ARGS&&... args) {
			if constexpr(Countable) ++NUM_EXISTING_BASE::num_existing;
			return Accessor<MUTAB>(this, internal::list_emplace(_alloc(), where, std::forward<ARGS>(args)...));
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


		inline auto end() {
			return Iterator<MUTAB>(this, _back);
		}

		inline auto end() const {
			return Iterator<CONST>(this, _back);
		}

	};







	struct With_Builder : List {
		FORWARDING_CONSTRUCTOR(With_Builder, List) {}
		FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(With_Builder, List) {}

		template<class NEW_ALLOCATOR>
		using ALLOCATOR =
			typename Context<Val, NEW_ALLOCATOR, Countable> :: With_Builder;

		using COUNTABLE =
			typename Context<Val, Allocator, true> :: With_Builder;

		using FULL_BLOWN =
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
	Vector_Allocator<VAL>,
	false // COUNTABLE
> :: With_Builder;











}
