#pragma once

#include "list.hpp"


namespace salgo {






namespace internal {
namespace Hash_Table {



template<class T> struct Add_val {
	T val;

	template<class TT>
	Add_val(TT&& v) : val( std::forward<TT>(v) ) {}
};
template<> struct Add_val<void> {};



template<class _KEY, class _VAL, class _HASH>
struct Context {

	//
	// forward declarations
	//
	struct Node;
	template<Const_Flag C> class Accessor;
	template<Const_Flag C> class Iterator;
	class Hash_Table;


	//
	// template arguments
	//
	using Key = _KEY;
	using Val = _VAL;
	using Hash = _HASH;

	static constexpr bool Has_Val = !std::is_same_v<Val, void>;

	using List = salgo::List<Node>;

	using Allocator = List :: Allocator;


	using       Handle = typename List::      Handle;
	using Small_Handle = typename List::Small_Handle;




	struct Node : Add_val<Val> {
		const Key key;

		// 1 or 2 arguments
		template<class K, class... V>
		Node(K&& k, V&&... v) : Add_val<Val>( std::forward<V>(v)... ), key( std::forward<K>(k) ) {}
	};






	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		// get handle
		operator Handle() const { return _handle; }
		auto     handle() const { return _handle; }

		// get val
		auto& operator()()       { return _owner[ _handle ]; }
		auto& operator()() const { return _owner[ _handle ]; }
		operator auto&()       { return operator()(); }
		operator auto&() const { return operator()(); }

		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.erase( _handle );
		}

		bool exists() const {
			return _handle != *_owner._list.end();
		}


	private:
		Accessor(Const<Hash_Table,C>& owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Hash_Table;
		friend Iterator<C>;


	private:
		Const<Hash_Table,C>& _owner;
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
			_handle = _owner->_buckets[0].next( _handle );
			DCHECK( _handle.valid() ) << "followed broken list link";
		}

		inline void _decrement() {
			_handle = _owner->_buckets[0].prev( _handle );
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
		inline Iterator(Const<Hash_Table,C>* owner, Handle handle) : _owner(owner), _handle(handle) {}
		friend Hash_Table;

	private:
		Const<Hash_Table,C>* _owner;
		Handle _handle;
	};








	class Hash_Table : private Hash, private Allocator {

	public:
		using Key = Context::Key;
		using Val = Context::Val;

		using Handle = Context::Handle;
		using Small_Handle = Context::Small_Handle;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;

		friend Iterator<MUTAB>;
		friend Iterator<CONST>;

	public:
		Hash_Table() = default;

		template<class H>
		Hash_Table(H&& hash) : Hash( std::forward<H>(hash) ) {}

		//
		// data
		//
	private:
		salgo::Memory_Block< salgo::Unordered_Vector > :: DENSE  _buckets;
		int _count = 0;

	private:
		auto& _alloc()       { return *static_cast<Allocator*>(this); }
		auto& _alloc() const { return *static_cast<Allocator*>(this); }

		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		void erase(Handle handle) {
			DCHECK( handle.valid() );
			DCHECK_GT( _count, 0 );
			DCHECK_GT( _buckets.size(), 0 );

			// lists need to have common allocator
			list_erase(_alloc(), handle);
			--_count;
		}




		auto& operator[](Handle handle) {
			DCHECK( handle.valid() );
			DCHECK_GT( _count, 0 );
			DCHECK_GT( _buckets.size(), 0 );

			if constexpr(Has_Val) return list_get(_alloc(), handle).val;
			else return list_get(_alloc(), handle).key;
		}

		auto& operator[](Handle handle) const {
			DCHECK( handle.valid() );
			DCHECK_GT( _count, 0 );
			DCHECK_GT( _buckets.size(), 0 );

			if constexpr(Has_Val) return list_get(_alloc(), handle).val;
			else return list_get(_alloc(), handle).key;
		}




		auto operator()(Handle handle)       { return Accessor<MUTAB>(*this, handle); }
		auto operator()(Handle handle) const { return Accessor<CONST>(*this, handle); }


		auto& operator[](const Key& k)       { return operator[]( _find<false>(k) ); }
		auto& operator[](const Key& k) const { return operator[]( _find<false>(k) ); }

		auto operator()(const Key& k)       { return operator()( _find<true>(k) ); }
		auto operator()(const Key& k) const { return operator()( _find<true>(k) ); }


	private:
		Handle _find(const Key& k) const {
			if(_buckets.size() == 0) return Handle();

			for(auto e : _buckets[k]) {
				if(e().key == k) {
					return e.handle();
				}
			}

			return Handle();
		}



	public:
		template<class K, class... V>
		auto emplace(K&& k, V&&... v) {
			++_count;

			// re-bucket
			int want_buckets = _count;
			if(_buckets.size()*3/2 < want_buckets) {
				_buckets.resize(want_buckets);
			}

			return Accessor<MUTAB>(*this, new_element);
		}

		int count() const {
			return _count;
		}





	public:
		inline auto begin() {
			return Iterator<MUTAB>(this, *_list.begin());
		}

		inline auto begin() const {
			return Iterator<CONST>(this, *_list.begin());
		}

		inline auto cbegin() const {
			return Iterator<CONST>(this, *_list.begin());
		}


		inline auto end() {
			return Iterator<MUTAB>(this, *_list.end());
		}

		inline auto end() const {
			return Iterator<CONST>(this, *_list.end());
		}

		inline auto cend() const {
			return Iterator<CONST>(this, *_list.end());
		}

	};







	struct With_Builder : Hash_Table {
		FORWARDING_CONSTRUCTOR(With_Builder, Hash_Table);

		template<class NEW_HASH>
		using HASH = typename Context<Key, Val, NEW_HASH> :: With_Builder;
	};




}; // struct Context
} // namespace Hash_Table
} // namespace internal






template<
	class KEY,
	class VAL = void
>
using Hash_Table = typename internal::Hash_Table::Context<
	KEY,
	VAL,
	::std::hash<KEY>
> :: With_Builder;











}
