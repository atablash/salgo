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



#include "helper-macros-on"
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
		bool _just_erased = false;

	protected:
		bool just_erased() const { return _just_erased; }

		void on_erase() {
			_next = _container()._alloc()[_handle()].next;
			_prev = _container()._alloc()[_handle()].prev;
			_just_erased = true;
		}

		void reset() {
			_prev.reset();
			_next.reset();
			_just_erased = false;
		}

		auto get_next() {
			if(_just_erased) return _next;
			else return _container()._alloc()[ _handle() ].next;
		}

		auto get_prev() {
			if(_just_erased) return _prev;
			else return _container()._alloc()[ _handle() ].prev;
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

			auto prv = CO._alloc()[HA].prev;
			auto nxt = CO._alloc()[HA].next;
			if(prv.valid()) CO._alloc()[prv].next = nxt;
			if(nxt.valid()) CO._alloc()[nxt].prev = prv;

			if(_container()._front == HA) _container()._front = nxt;
			if(_container()._back  == HA) _container()._back  = prv;

			ALLOC(HA).destruct();

			if constexpr(Countable) --_container().num_existing;
		}

		template<class... ARGS>
		auto emplace_before(ARGS&&... args) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( HA.valid() && !BASE::just_erased());

			auto new_node = _container()._alloc().construct( std::forward<ARGS>(args)... );

			new_node().prev = BASE::get_prev();
			if(new_node().prev.valid()) _container()._alloc()[ new_node().prev ].next = new_node.handle();

			new_node().next = HA;
			_container()._alloc()[ HA ].prev = new_node.handle();

			if(_container()._front == HA) _container()._front = new_node.handle();

			if constexpr(Countable) ++_container().num_existing;
			return Accessor<MUTAB>(&_container(), new_node);
		}

		template<class... ARGS>
		auto emplace_after(ARGS&&... args) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( HA.valid() && !BASE::just_erased());

			auto new_node = _container()._alloc().construct( std::forward<ARGS>(args)... );

			new_node().next = BASE::get_next();
			if(new_node().next.valid()) _container()._alloc()[ new_node().next ].prev = new_node.handle();

			new_node().prev = HA;
			_container()._alloc()[ HA ].next = new_node.handle();

			if(_container()._back == HA) _container()._back = new_node.handle();

			if constexpr(Countable) ++_container().num_existing;
			return Accessor<MUTAB>(&_container(), new_node);
		}


		auto next()       { return _container()( BASE::get_next() ); }
		auto next() const { return _container()( BASE::get_next() ); }

		auto prev()       { return _container()( BASE::get_prev() ); }
		auto prev() const { return _container()( BASE::get_prev() ); }
	};




	class End_Iterator {};



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
			DCHECK( _handle().valid() ) << "followed broken list link";
			_handle() = BASE::get_next();
			BASE::reset();
		}

		void _decrement() {
			DCHECK( _handle().valid() ) << "followed broken list link";
			_handle() = BASE::get_prev();
			BASE::reset();
		}

	public:
		bool operator!=(End_Iterator) const { return _handle().valid(); }
	};















	struct Node {
		Val val;

		FORWARDING_CONSTRUCTOR(Node, val) {}

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
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;

		friend Iterator<MUTAB>;
		friend Iterator<CONST>;

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
		List() = default;

		List(int size) {
			for(int i=0; i<size; ++i) {
				emplace_back();
			}
		}

		~List() {
			for(auto& e : *this) e.erase(); // todo: erase faster, without managing links
		}

		void clear() {
			for(auto& e : *this) e.erase(); // todo: erase faster, without managing links

			_front.reset();
			_back.reset();
		}






		// copy
		//List(const List& o) {
		//	*this = o;
		//}

		// trivial move
		//List(List&& o) = default;

		// copy assignment
		//List& operator=(const List& o) {
		//	clear();
		//	*this += o;
		//}

		// move assignment: todo

		//List& operator+=(const List& o) {
		//	for(auto e : o) {
		//		emplace_back( e() );
		//	}
		//}

		// todo
		//List& operator+=(List&& o) {}




		//bool exists(Handle handle) const {
		//	return v[ key ].exists;
		//}







		// direct access
	public:
		auto& operator[](Handle handle)       { return _alloc()[handle].val; }
		auto& operator[](Handle handle) const { return _alloc()[handle].val; }

		auto& operator[](First_Tag)       { return operator[](_front); }
		auto& operator[](First_Tag) const { return operator[](_front); }

		auto& operator[](Last_Tag)       { return operator[](_back); }
		auto& operator[](Last_Tag) const { return operator[](_back); }




		// accessor access
	public:
		auto operator()(Handle handle)       { return Accessor<MUTAB>(this, handle); }
		auto operator()(Handle handle) const { return Accessor<CONST>(this, handle); }

		auto operator()(First_Tag)       { return operator()(_front); }
		auto operator()(First_Tag) const { return operator()(_front); }

		auto operator()(Last_Tag)       { return operator()(_back); }
		auto operator()(Last_Tag) const { return operator()(_back); }







	public:
		template<class... ARGS>
		auto emplace_front(ARGS&&... args) {
			if(empty()) return _emplace_into_empty( std::forward<ARGS>(args)... );
			return (*this)(FIRST).emplace_before( std::forward<ARGS>(args)... );
		}

		template<class... ARGS>
		auto emplace_back(ARGS&&... args) {
			if(empty()) return _emplace_into_empty( std::forward<ARGS>(args)... );
			return (*this)(LAST).emplace_after( std::forward<ARGS>(args)... );
		}


	private:
		template<class... ARGS>
		auto _emplace_into_empty(ARGS&&... args) {
			DCHECK(!_front.valid());
			DCHECK(!_back.valid());
			_front = _back = _alloc().construct( std::forward<ARGS>(args)... ).handle();

			if constexpr(Countable) {
				++NUM_EXISTING_BASE::num_existing;
				DCHECK_EQ(1, NUM_EXISTING_BASE::num_existing);
			}

			return Accessor<MUTAB>(this, _front);
		}







	public:
		int count() const {
			static_assert(Countable, "count() called on non countable Sparse_Vector");
			return NUM_EXISTING_BASE::num_existing;
		}

		bool empty() const { return !_front.valid(); }




	public:
		auto begin() {
			return Iterator<MUTAB>(this, _front);
		}

		auto begin() const {
			return Iterator<CONST>(this, _front);
		}


		auto end() const { return End_Iterator(); }

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











} // namespace salgo
#include "helper-macros-off"


