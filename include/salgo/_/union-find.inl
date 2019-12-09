#pragma once

#include "union-find.hpp"

#include "dynamic-array.inl"

#include "add-member.hpp"
#include "inplace-storage.hpp"

#include "helper-macros-on.inc"

namespace salgo::_::union_find {


SALGO_GENERATE_HAS_MEMBER(merge_with);


template<class DATA, bool COUNTABLE, bool COUNTABLE_SETS>
struct Params {
	using Data = DATA;
	static constexpr bool Countable = COUNTABLE;
	static constexpr bool Countable_Sets = COUNTABLE_SETS;

	static constexpr bool Has_Data = !std::is_same_v<Data, void>;

	using Node = union_find::Node<Params>;

	using Allocator = salgo::Dynamic_Array<Node>;

	using Handle       = typename Allocator::Handle;
	using Handle_Small = typename Allocator::Handle_Small;
	using Index        = typename Allocator::Index;
};



SALGO_ADD_MEMBER_STORAGE(data);
SALGO_ADD_MEMBER(_count);



template<class P>
struct Node :
		Add_Storage_data<typename P::Data, P::Has_Data>,
		Add__count<int, P::Countable_Sets> {

	using DATA_BASE = Add_Storage_data<typename P::Data, P::Has_Data>;
	using COUNT_BASE = Add__count<int, P::Countable_Sets>;

	Node() { if constexpr(P::Has_Data) DATA_BASE::data.construct(); }

	Node(Node&& o) = default;

	template<class... ARGS>
	Node(ARGS&&... args) {
		static_assert(P::Has_Data, "if no data, only 0-argument constructor can be used");
		DATA_BASE::data.construct( std::forward<ARGS>(args)... );
		if constexpr(P::Countable_Sets) COUNT_BASE::count = 1;
	}

	mutable typename P::Handle_Small parent;

	~Node() {
		if constexpr(P::Has_Data) {
			if(!parent.valid()) DATA_BASE::data.destruct();
		}
	}


	// use Data::merge_with(Data&) if available
	// otherwise use operator+=
	void merge_with(Node& b) {
		if constexpr(P::Has_Data) {
			if constexpr( has_member__merge_with<typename P::Data> ) {
				DATA_BASE::data.get().merge_with( b.data.get() );
			}
			else {
				b.data.get() += DATA_BASE::data.get();
			}

			DATA_BASE::data.destruct();
		}

		if constexpr(P::Countable_Sets) b.count += COUNT_BASE::count;
	}
};




template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C,Context<P>> {
	using BASE = Accessor_Base<C,Context<P>>;
	using BASE::BASE;

public:
	auto& merge_with(typename P::Index idx) {
		_update();
		auto b = CONT(idx);

		if(*this == b) return *this; // todo: do we want this check?

		static_assert(C==MUTAB, "called union_with() on const accessor");
		DCHECK( !NODE.parent.valid() ) <<
			"can't merge this accessor, because it's already invalidated. "
			"please request a new accessor from the Union_Find object";

		DCHECK_EQ( b, CONT( b.handle() ) ) <<
			"can't merge with this accessor, because it's already invalidated. "
				"please request a new accessor for the merge_with() target, from the Union_Find object";

		NODE.parent = b;

		if constexpr(P::Countable_Sets) {
			DCHECK_LE(NODE.count, b.count()) <<
				"a.merge_with(b): can't merge bigger to smaller. "
				"either use b.merge_with(a), or use Union_Find::merge(a,b) "
				"to check for sizes during runtime (if have Countable_Sets flag)";
		}

		NODE.merge_with( ALLOC[ b.handle() ] );

		//ALLOC( HANDLE ).erase();

		MUT_HANDLE = b.handle();

		if constexpr(P::Countable) {
			--CONT._count;
		}

		return *this;
	}

	template<Const_Flag CC>
	bool operator==(const Accessor<P,CC>& o) const { _update(); o._update(); return BASE::operator==(o); }

	template<Const_Flag CC>
	bool operator!=(const Accessor<P,CC>& o) const { _update(); o._update(); return BASE::operator!=(o); }

private:
	// todo optim: check if calling _update() all the time hurts performance
	void _update() const { *const_cast<typename P::Handle*>(&HANDLE) = CONT._find(HANDLE); }
};



template<class P>
struct End_Iterator {};



template<class P, Const_Flag C>
class Iterator : public Iterator_Base<C,Context<P>> {
	using BASE = Iterator_Base<C,Context<P>>;

public:
	using BASE::BASE;

public:
	bool operator!=(End_Iterator<P>) { return ALLOC(HANDLE).iterator() != ALLOC.end(); }
};







template<class P>
struct Context {
	using Container = Union_Find<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Accessor = union_find::Accessor<P,C>;

	template<Const_Flag C>
	using Iterator = union_find::Accessor<P,C>;
};











template<class P>
class Union_Find : protected P,
		private P::Allocator,
		private Add__count<int, P::Countable> {

	using COUNT_BASE = Add__count<int, P::Countable>;

	using typename P::Index;
	using typename P::Handle;

public:
	template<Const_Flag C> using Accessor = union_find::Accessor<P,C>;
	template<Const_Flag C> using Iterator = union_find::Iterator<P,C>;

private:
	friend Accessor<CONST>;
	friend Accessor<MUTAB>;
	friend Iterator<CONST>;
	friend Iterator<MUTAB>;


public:
	Union_Find() = default;

	template<class... ARGS>
	Union_Find(int initial_size, ARGS&&... args) : P::Allocator(initial_size, std::forward<ARGS>(args)...) {
		if constexpr(P::Countable) COUNT_BASE::_count = initial_size;
	}


public:
	auto& operator[](Index handle)       { (void)handle; // suppress unused warning
		if constexpr(P::Has_Data) return _alloc()[ _find(handle) ].data.get();
		else return *this; // never reached
	}

	auto& operator[](Index handle) const { (void)handle; // suppress unused warning
		if constexpr(P::Has_Data) return _alloc()[ _find(handle) ].data.get();
		else return *this; // never reached
	}


	auto operator()(Index handle)       { return Accessor<MUTAB>(this, _find(handle)); }
	auto operator()(Index handle) const { return Accessor<CONST>(this, _find(handle)); }



public:
	template<class... ARGS>
	auto add(ARGS&&... args) {
		if constexpr(P::Countable) ++COUNT_BASE::_count;
		auto handle = _alloc().add( std::forward<ARGS>(args)... ).handle();
		return Accessor<MUTAB>(this, handle);
	}

	auto merge(Index _a, Index b) {
		auto a = operator()(_a);

		if constexpr(P::Countable_Sets) {
			if(_alloc()[a].count > _alloc()[b].count) std::swap(a,b);
		}

		a.merge_with( b );

		return Accessor<MUTAB>(this, b);
	}


	auto domain() const { return _alloc().domain(); }
	auto count()  const {
		static_assert(P::Countable, "count unknown if not Countable");
		if constexpr(P::Countable) return COUNT_BASE::_count;
		else return nullptr; // never reached
	}


private:
	auto _find(Handle handle) const {
		Handle root = handle;
		while(_alloc()[root].parent.valid()) root = _alloc()[root].parent;

		while(handle != root) {
			auto parent = _alloc()[handle].parent;
			_alloc()[handle].parent = root;
			handle = parent;
		}

		return handle;
	}


public:
	auto begin()       { return Iterator<MUTAB>(this, _alloc().begin()); }
	auto begin() const { return Iterator<CONST>(this, _alloc().begin()); }

	auto end()       { return End_Iterator<P>(); }
	auto end() const { return End_Iterator<P>(); }


private:
	auto& _alloc()       { return *(      typename P::Allocator*)(this); }
	auto& _alloc() const { return *(const typename P::Allocator*)(this); }
};





template<class P>
class With_Builder : public Union_Find<P> {
	using BASE = Union_Find<P>;

public:
	using BASE::BASE;

	using typename P::Data;
	using P::Countable;
	using P::Countable_Sets;

	template<class X>
	using DATA = With_Builder< Params<X, Countable, Countable_Sets>>;

	using COUNTABLE = With_Builder< Params<Data, true, Countable_Sets>>;

	using COUNTABLE_SETS = With_Builder< Params<Data, Countable, true>>;
};


} // namespace salgo::_::union_find

#include "helper-macros-off.inc"

