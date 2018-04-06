#pragma once

#include "common.hpp"
#include "const-flag.hpp"
#include "stack-storage.hpp"
#include "iterator-base.hpp"
#include "int-handle.hpp"



namespace salgo {






namespace internal {
namespace Memory_Block {


template<bool> struct Add_num_existing { int num_existing = 0; };
template<> struct Add_num_existing<false> {};


template<bool> struct Add_exists { bool exists = false; };
template<> struct Add_exists<false> {};



template<class _VAL, bool _ITERABLE, bool _COUNTABLE>
struct Context {

	using Val = _VAL;
	static constexpr bool Iterable = _ITERABLE;
	static constexpr bool Countable = _COUNTABLE;

	//
	// forward declarations
	//

	struct Node;
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Memory_Block;





	struct Handle : Int_Handle<Handle> { FORWARDING_CONSTRUCTOR(Handle,Int_Handle<Handle>); };



	struct Node : Add_exists<Iterable> {
		Stack_Storage<Val> val;

		~Node() {
			if constexpr(Iterable) if(Add_exists<Iterable>::exists) val.destruct();
		}
	};






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		Handle handle() const {
			return _key;
		}

		Const<Val,C>& val() {
			if constexpr(Iterable) DCHECK( _owner.v[ _key ].exists ) << "accessing erased element";
			return _owner[ _key ];
		}

		const Val& val() const {
			if constexpr(Iterable) DCHECK( _owner.v[ _key ].exists ) << "accessing erased element";
			return _owner[ _key ];
		}

		template<class... ARGS>
		void construct(ARGS&&... args) {
			static_assert(C == MUTAB, "called construct() on CONST accessor");
			_owner.construct( _key, std::forward<ARGS>(args)... );
		}

		void destruct() {
			static_assert(C == MUTAB, "called destruct() on CONST accessor");
			_owner.destruct( _key );
		}

		bool exists() const {
			return _owner.exists( _key );
		}


	private:
		Accessor(Const<Memory_Block,C>& owner, Handle key)
			: _owner(owner), _key(key) {}

		friend Memory_Block;
		friend Iterator<C>;


	private:
		Const<Memory_Block,C>& _owner;
		const Handle _key;
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
		inline Iterator(Const<Memory_Block,C>& owner, Handle key)
				: _owner(owner), _key(key) {
			if((int)key != owner.domain() && !owner.v[key].exists) _increment();
		}

		friend Memory_Block;

	private:
		Const<Memory_Block,C>& _owner;
		int _key;
	};








	class Memory_Block : private Add_num_existing<Countable> {
		using NUM_EXISTING_BASE = Add_num_existing<Countable>;

	public:
		using Val = Context::Val;
		static constexpr bool Is_Iterable = Context::Iterable;
		static constexpr bool Is_Countable = Context::Countable;

		using Handle = Context::Handle;


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
		Memory_Block() = default;
		Memory_Block(int size) : v(size) {
			// if constexpr(Countable) NUM_EXISTING_BASE::num_existing = size; // uninitialized at first!
		}



		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		template<class... ARGS>
		void construct(Handle key, ARGS&&... args) {
			_check_bounds(key);
			if constexpr(Iterable) {
				DCHECK(!v[key].exists) << "element already constructed";
				v[key].exists = true;
			}
			v[key].val.construct( std::forward<ARGS>(args)... );
			if constexpr(Countable) ++NUM_EXISTING_BASE::num_existing;
		}

		void destruct(Handle key) {
			_check_bounds(key);
			if constexpr(Iterable) {
				DCHECK( v[key].exists ) << "erasing already erased element";
				v[key].exists = false;
			}
			v[key].val.destruct();
			if constexpr(Countable) --NUM_EXISTING_BASE::num_existing;
		}

		bool exists(Handle key) const {
			_check_bounds(key);
			return v[key].exists;
		}

		Val& operator[](Handle key) {
			_check_bounds(key);
			if constexpr(Iterable) DCHECK( v[key].exists ) << "accessing non-existing element";
			return v[key].val;
		}

		const Val& operator[](Handle key) const {
			_check_bounds(key);
			if constexpr(Iterable) DCHECK( v[key].exists ) << "accessing non-existing element";
			return v[key].val;
		}


		template<class... ARGS>
		void construct_all(const ARGS&... args) {
			for(int i=0; i<(int)v.size(); ++i) {
				if constexpr(Iterable) {
					DCHECK(!v[i].exists) << "can't construct_all() if some elements already exist";
				}
				construct(i, args...);
			}
		}





	private:
		inline void _check_bounds(Handle key) const {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
		}





		//
		// interface
		//
	public:
		auto operator()(Handle key) {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
			return Accessor<MUTAB>(*this, key);
		}

		auto operator()(Handle key) const {
			DCHECK_GE( key, 0 ) << "index out of bounds";
			DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
			return Accessor<CONST>(*this, key);
		}

		template<class... ARGS>
		Accessor<MUTAB> emplace_back(ARGS&&... args) {
			// because of vector realloc:
			static_assert( Iterable || std::is_trivially_copy_constructible_v<Val>,
				"non-trivially-constructible types require ITERABLE to emplace_back()" );

			v.emplace_back();
			v.back().val.construct( std::forward<ARGS>(args)... );
			if constexpr(Iterable) v.back().exists = true;

			if constexpr(Countable) ++NUM_EXISTING_BASE::num_existing;
			return Accessor<MUTAB>(*this, v.size()-1);
		}


		int count() const {
			static_assert(Countable, "count() called on non countable Memory_Block");
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
			static_assert(Iterable, "can only compact if ITERABLE");

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
			static_assert(Iterable, "not ITERABLE");
			return Iterator<MUTAB>(*this, 0);
		}

		inline auto begin() const {
			static_assert(Iterable, "not ITERABLE");
			return Iterator<CONST>(*this, 0);
		}

		inline auto cbegin() const {
			static_assert(Iterable, "not ITERABLE");
			return Iterator<CONST>(*this, 0);
		}


		inline auto end() {
			static_assert(Iterable, "not ITERABLE");
			return Iterator<MUTAB>(*this, v.size());
		}

		inline auto end() const {
			static_assert(Iterable, "not ITERABLE");
			return Iterator<CONST>(*this, v.size());
		}

		inline auto cend() const {
			static_assert(Iterable, "not ITERABLE");
			return Iterator<CONST>(*this, v.size());
		}

	};







	struct With_Builder : Memory_Block {
		FORWARDING_CONSTRUCTOR(With_Builder, Memory_Block);

		using ITERABLE =
			typename Context< Val, true, Countable > :: With_Builder;

		using COUNTABLE =
			typename Context< Val, Iterable, true > :: With_Builder;

		using FULL =
			typename Context< Val, true, true > :: With_Builder;
	};




}; // struct Context
} // namespace Memory_Block
} // namespace internal






template<
	class T
>
using Memory_Block = typename internal::Memory_Block::Context<
	T,
	false, // ITERABLE
	false  // COUNTABLE
> :: With_Builder;











}
