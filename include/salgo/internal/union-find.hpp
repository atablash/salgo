#pragma once

#include "common.hpp"
#include "vector.hpp"
#include "inplace-storage.hpp"

#include "helper-macros-on"

namespace salgo {

namespace internal {

namespace union_find {


ADD_MEMBER_STORAGE(data);
ADD_MEMBER(_count);

GENERATE_HAS_MEMBER(merge_with);


template<class _DATA, bool _COUNTABLE, bool _COUNTABLE_SETS>
struct Context {

	using Data = _DATA;
	static constexpr bool Countable = _COUNTABLE;
	static constexpr bool Countable_Sets = _COUNTABLE_SETS;


	static constexpr bool Has_Data = !std::is_same_v<Data, void>;




	struct Node;



	using Allocator = salgo::Vector<Node>;

	using Handle       = typename Allocator::Handle;
	using Handle_Small = typename Allocator::Handle_Small;
	using Index        = typename Allocator::Index;





	struct Node :
			Add_Storage_data<Data, Has_Data>,
			Add__count<int, Countable_Sets> {

		using DATA_BASE = Add_Storage_data<Data, Has_Data>;
		using COUNT_BASE = Add__count<int, Countable_Sets>;

		Node() { if constexpr(Has_Data) DATA_BASE::data.construct(); }

		Node(Node&& o) = default;

		template<class... ARGS>
		Node(ARGS&&... args) {
			static_assert(Has_Data, "if no data, only 0-argument constructor can be used");
			DATA_BASE::data.construct( std::forward<ARGS>(args)... );
			if constexpr(Countable_Sets) COUNT_BASE::count = 1;
		}

		mutable Handle_Small parent;

		~Node() {
			if constexpr(Has_Data) {
				if(!parent.valid()) DATA_BASE::data.destruct();
			}
		}


		// use Data::merge_with(Data&) if available
		// otherwise use operator+=
		void merge_with(Node& b) {
			if constexpr(Has_Data) {
				if constexpr( has_member__merge_with<Data> ) {
					DATA_BASE::data.get().merge_with( b.data.get() );
				}
				else {
					b.data.get() += DATA_BASE::data.get();
				}

				DATA_BASE::data.destruct();
			}

			if constexpr(Countable_Sets) b.count += COUNT_BASE::count;
		}
	};






	class Union_Find;
	using Container = Union_Find;





	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;

	public:
		FORWARDING_CONSTRUCTOR_2(Accessor, BASE) {}

	public:
		auto& merge_with(const Accessor& b) {
			static_assert(C==MUTAB, "called union_with() on const accessor");
			DCHECK( !NODE.parent.valid() ) <<
				"can't merge this accessor, because it's already invalidated. "
				"please request a new accessor from the Union_Find object";

			DCHECK_EQ( b, CONT( b.handle() ) ) <<
				"can't merge with this accessor, because it's already invalidated. "
				 "please request a new accessor for the merge_with() target, from the Union_Find object";

			NODE.parent = b;

			if constexpr(Countable_Sets) {
				DCHECK_LE(NODE.count, b.count()) <<
					"a.merge_with(b): can't merge bigger to smaller. "
	 				"either use b.merge_with(a), or use Union_Find::merge(a,b) "
	  				"to check for sizes during runtime (if have Countable_Sets flag)";
			}

			NODE.merge_with( ALLOC[ b.handle() ] );

			//ALLOC( HANDLE ).erase();

			MUT_HANDLE = b.handle();

			if constexpr(Countable) {
				--CONT._count;
			}

			return *this;
		}

		auto& merge_with(Index idx) { return merge_with( CONT(idx) ); }

	public:
		template<Const_Flag CC>
		bool operator==(const Accessor<CC>& o) const { DCHECK_EQ(&CONT, &o.CONT); return HANDLE == o.HANDLE; }

		template<Const_Flag CC>
		bool operator!=(const Accessor<CC>& o) const { DCHECK_EQ(&CONT, &o.CONT); return HANDLE != o.HANDLE; }
	};




	struct End_Iterator {};



	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;

	public:
		FORWARDING_CONSTRUCTOR_2(Iterator, BASE) {}

	public:
		bool operator!=(End_Iterator) { return ALLOC(HANDLE).iterator() != ALLOC.end(); }
	};











	class Union_Find : private Allocator, private Add__count<int, Countable> {
		using COUNT_BASE = Add__count<int, Countable>;

	public:
		template<Const_Flag C> using Accessor = Context::Accessor<C>;
		template<Const_Flag C> using Iterator = Context::Iterator<C>;

	private:
		friend Accessor<CONST>;
		friend Accessor<MUTAB>;
		friend Iterator<CONST>;
		friend Iterator<MUTAB>;


	public:
		Union_Find() = default;

		template<class... ARGS>
		Union_Find(int initial_size, ARGS&&... args) : Allocator(initial_size, std::forward<ARGS>(args)...) {
			if constexpr(Countable) COUNT_BASE::_count = initial_size;
		}


	public:
		auto& operator[](Index handle)       { (void)handle; // suppress unused warning
			//static_assert(Has_Data);
			if constexpr(Has_Data) return _alloc()[ _find(handle) ].data.get();
			else return *this; // never reached
		}

		auto& operator[](Index handle) const { (void)handle; // suppress unused warning
			if constexpr(Has_Data) return _alloc()[ _find(handle) ].data.get();
			else return *this; // never reached
		}


		auto operator()(Index handle)       { return Accessor<MUTAB>(this, _find(handle)); }
		auto operator()(Index handle) const { return Accessor<CONST>(this, _find(handle)); }



	public:
		template<class... ARGS>
		auto add(ARGS&&... args) {
			if constexpr(Countable) ++COUNT_BASE::_count;
			auto handle = _alloc().add( std::forward<ARGS>(args)... ).handle();
			return Accessor<MUTAB>(this, handle);
		}

		auto merge(Accessor<MUTAB>& a, Index b) {
			if constexpr(Countable_Sets) {
				if(_alloc()[a].count > _alloc()[b].count) std::swap(a,b);
			}

			a.merge_with( b );

			return Accessor<MUTAB>(this, b);
		}

		auto merge(Index a, Index b) { return merge(operator()(a), b); }


		auto domain() const { return _alloc().domain(); }
		auto count()  const {
			static_assert(Countable, "count unknown if not Countable");
			if constexpr(Countable) return COUNT_BASE::_count;
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

		auto end()       { return End_Iterator(); }
		auto end() const { return End_Iterator(); }


	private:
		auto& _alloc()       { return *(      Allocator*)(this); }
		auto& _alloc() const { return *(const Allocator*)(this); }
	};














	struct With_Builder : Union_Find {
		FORWARDING_CONSTRUCTOR(With_Builder, Union_Find) {}

		template<class X>
		using DATA = typename Context<X, Countable, Countable_Sets> ::With_Builder;

		using COUNTABLE = typename Context<Data, true, Countable_Sets> ::With_Builder;

		using COUNTABLE_SETS = typename Context<Data, Countable, true> ::With_Builder;
	};

};


} // namespace union_find

} // namespace internal





using Union_Find = internal::union_find::Context<
	void, // DATA
	false, // COUNTABLE
	false // COUNTABLE_SETS
>::With_Builder;




} // namespace salgo


#include "helper-macros-off"

