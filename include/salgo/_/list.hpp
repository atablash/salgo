#pragma once

#include "alloc/array-allocator.hpp"

#include "add-member.hpp"
#include "inplace-storage.hpp"
#include "accessors.hpp"
#include "const-flag.hpp"
#include "subscript-tags.hpp"
#include "iterable-base.hpp"


#ifndef NDEBUG
#include <unordered_set>
#endif


#include "helper-macros-on.inc"

namespace salgo::_::List {


SALGO_ADD_MEMBER(num_existing);









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
	using Handle_Small = typename Allocator :: Handle_Small;









	template<Const_Flag C>
	class Reference : public Reference_Base<C,Context> {
		using BASE = Reference_Base<C,Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Reference, BASE) {}

	private:
		Handle_Small _prev;
		Handle_Small _next;
		bool _just_erased = false;

	protected:
		bool just_erased() const { return _just_erased; }

		void on_erase() {
			_next = ALLOC[HANDLE].next;
			_prev = ALLOC[HANDLE].prev;
			_just_erased = true;
		}

		void reset() {
			_prev.reset();
			_next.reset();
			_just_erased = false;
		}

		auto get_next() {
			if(_just_erased) return _next;
			else return ALLOC[ HANDLE ].next;
		}

		auto get_prev() {
			if(_just_erased) return _prev;
			else return ALLOC[ HANDLE ].prev;
		}
	};


	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Accessor,BASE) {}

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			BASE::on_erase();

			auto prv = NODE.prev;
			auto nxt = NODE.next;
			if(prv.valid()) ALLOC[prv].next = nxt;
			if(nxt.valid()) ALLOC[nxt].prev = prv;

			if(CONT._front == HANDLE) CONT._front = nxt;
			if(CONT._back  == HANDLE) CONT._back  = prv;

			ALLOC( HANDLE ).destruct();

			if constexpr(Countable) --CONT.num_existing;
		}

		template<class... ARGS>
		auto emplace_before(ARGS&&... args) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( HANDLE.valid() && !BASE::just_erased());

			auto new_node = ALLOC.construct( std::forward<ARGS>(args)... );

			new_node().prev = BASE::get_prev();
			if(new_node().prev.valid()) ALLOC[ new_node().prev ].next = new_node.handle();

			new_node().next = HANDLE;
			NODE.prev = new_node.handle();

			if(CONT._front == HANDLE) CONT._front = new_node.handle();

			if constexpr(Countable) ++CONT.num_existing;
			return Accessor<MUTAB>(&CONT, new_node);
		}

		template<class... ARGS>
		auto emplace_after(ARGS&&... args) {
			static_assert(C == MUTAB, "called on CONST accessor");
			DCHECK( HANDLE.valid() && !BASE::just_erased());

			auto new_node = ALLOC.construct( std::forward<ARGS>(args)... );

			new_node().next = BASE::get_next();
			if(new_node().next.valid()) ALLOC[ new_node().next ].prev = new_node.handle();

			new_node().prev = HANDLE;
			NODE.next = new_node.handle();

			if(CONT._back == HANDLE) CONT._back = new_node.handle();

			if constexpr(Countable) ++CONT.num_existing;
			return Accessor<MUTAB>(&CONT, new_node);
		}


		auto next()       { return CONT( BASE::get_next() ); }
		auto next() const { return CONT( BASE::get_next() ); }

		auto prev()       { return CONT( BASE::get_prev() ); }
		auto prev() const { return CONT( BASE::get_prev() ); }


		auto next_in_cycle()       {
			auto r = CONT( BASE::get_next() );
			return r.found() ? r : CONT(FIRST);
		}

		auto next_in_cycle() const {
			auto r = CONT( BASE::get_next() );
			return r.found() ? r : CONT(FIRST);
		}

		auto prev_in_cycle()       {
			auto r = CONT( BASE::get_prev() );
			return r.found() ? r : CONT(LAST);
		}

		auto prev_in_cycle() const {
			auto r = CONT( BASE::get_prev() );
			return r.found() ? r : CONT(LAST);
		}


		bool valid() const { return HANDLE.valid(); }
		bool not_valid() const { return ! valid(); }
	};




	class End_Iterator {};



	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;

	public:
		using BASE::BASE;
		// FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

	private:
		friend BASE;

		void _increment() {
			DCHECK( HANDLE.valid() ) << "followed broken list link";
			MUT_HANDLE = BASE::get_next();
			BASE::reset();
		}

		void _decrement() {
			DCHECK( HANDLE.valid() ) << "followed broken list link";
			MUT_HANDLE = BASE::get_prev();
			BASE::reset();
		}

	public:
		bool operator!=(End_Iterator) const { return HANDLE.valid(); }
	};















	struct Node {
		Val val;

		FORWARDING_CONSTRUCTOR_VAR(Node, val) {}

		Handle_Small next;
		Handle_Small prev;

		static constexpr bool Countable = Context::Countable;
	};








	class List :
			private Allocator,
			private Add_num_existing<int, Countable>,
			public Iterable_Base<List> {

		using NUM_EXISTING_BASE = Add_num_existing<int, Countable>;

	public:
		using Val = Context::Val;
		static constexpr bool Is_Countable = Context::Countable;

		using       Handle = Context::      Handle;
		using Handle_Small = Context::Handle_Small;

		template<Const_Flag C> using Accessor = Context::Accessor<C>;
		template<Const_Flag C> using Iterator = Context::Iterator<C>;


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

		List(std::initializer_list<Val> il) {
			for(auto& e : il) emplace_back(e);
		}

		~List() {
			for(auto& e : *this) e.erase(); // todo: erase faster, without managing links
		}

		void clear() {
			for(auto& e : *this) e.erase(); // todo: erase faster, without managing links

			_front.reset();
			_back.reset();
		}






	public:
		auto operator()(Handle handle)       { return Accessor<MUTAB>(this, handle); }
		auto operator()(Handle handle) const { return Accessor<CONST>(this, handle); }
		auto& operator[](Handle handle)       { return _alloc()[handle].val; }
		auto& operator[](Handle handle) const { return _alloc()[handle].val; }

		auto operator()(First_Tag)       { return operator()(_front); }
		auto operator()(First_Tag) const { return operator()(_front); }
		auto& operator[](First_Tag)       { return operator[](_front); }
		auto& operator[](First_Tag) const { return operator[](_front); }

		auto operator()(Last_Tag)       { return operator()(_back); }
		auto operator()(Last_Tag) const { return operator()(_back); }
		auto& operator[](Last_Tag)       { return operator[](_back); }
		auto& operator[](Last_Tag) const { return operator[](_back); }
	
		// todo: move this default implementation that delegates to operator[](First_Tag) to Iterable_Base
		auto operator()(Any_Tag)       { return operator()(FIRST); }
		auto operator()(Any_Tag) const { return operator()(FIRST); }
		auto& operator[](Any_Tag)       { return operator[](FIRST); }
		auto& operator[](Any_Tag) const { return operator[](FIRST); }

		void erase(Handle handle) { operator()(handle).erase(); }










	public:
		template<class... ARGS>
		auto emplace_front(ARGS&&... args) {
			if(is_empty()) return _emplace_into_empty( std::forward<ARGS>(args)... );
			return (*this)(FIRST).emplace_before( std::forward<ARGS>(args)... );
		}

		template<class... ARGS>
		auto emplace_back(ARGS&&... args) {
			if(is_empty()) return _emplace_into_empty( std::forward<ARGS>(args)... );
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
			static_assert(Countable, "count() called on non-countable List");
			return NUM_EXISTING_BASE::num_existing;
		}

		bool is_empty() const { return !_front.valid(); }
		bool not_empty() const { return !is_empty(); }




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
		using BASE = List;
		using BASE::BASE;

		// FORWARDING_CONSTRUCTOR(With_Builder, List) {}
		// FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(With_Builder, List) {}

		template<class NEW_ALLOCATOR>
		using ALLOCATOR =
			typename Context<Val, NEW_ALLOCATOR, Countable> :: With_Builder;

		using COUNTABLE =
			typename Context<Val, Allocator, true> :: With_Builder;

		using FULL_BLOWN =
			typename Context<Val, Allocator, true> :: With_Builder;
	};




}; // struct Context


} // salgo::_::List



#include "helper-macros-off.inc"





namespace salgo {

template<
	class VAL
>
using List = typename _::List::Context<
	VAL,
	::salgo::alloc::Array_Allocator<VAL>,
	false // COUNTABLE
> :: With_Builder;



} // namespace salgo


