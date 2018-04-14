#pragma once

#include "memory-block.hpp"
#include "unordered-vector.hpp"
#include "allocator.hpp"


namespace salgo {






namespace internal {
namespace hash_table {



template<class T> struct Add_val {
	T val;

	template<class TT>
	Add_val(TT&& v) : val( std::forward<TT>(v) ) {}
};
template<> struct Add_val<void> {};










template<class _KEY, class _VAL, class _HASH, class _ALLOCATOR, bool _INPLACE>
struct Context {

	//
	// template arguments
	//
	using Key = _KEY;
	using Val = _VAL;
	using Hash = _HASH;
	static constexpr bool Inplace = _INPLACE;

	static constexpr bool Has_Val = !std::is_same_v<Val, void>;





	struct Key_Val : Add_val<Val> {
		const Key key;

		// 1 or 2 arguments
		template<class K, class... V>
		Key_Val(K&& k, V&&... v) : Add_val<Val>( std::forward<V>(v)... ), key( std::forward<K>(k) ) {}
	};



	using Allocator = typename _ALLOCATOR :: template VAL<Key_Val>;


	using Node = std::conditional_t<
		Inplace,
		Key_Val,
		typename Allocator::Small_Handle
	>;

	using List = typename salgo::Unordered_Vector<Node> :: template STACK_BUFFER<5>;
	using Buckets = typename salgo::Memory_Block<List> :: DENSE;



	struct Handle {
		typename Buckets::Handle a;
		typename List::Handle b;

		bool valid() const { return a.valid(); }
	};

	using Small_Handle = Handle; // the same currently










	//
	// forward declarations
	//
	class Hash_Table;



	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Iterator_Base<C,Accessor> {
	public:
		// get handle
		auto     handle() const { return _handle; }
		operator Handle() const { return handle(); }

		// get val
		auto& operator()()       { return (*_owner)[ _handle ]; }
		auto& operator()() const { return (*_owner)[ _handle ]; }
		operator auto&()       { return operator()(); }
		operator auto&() const { return operator()(); }

		// get key
		auto& key()       { return _owner->key( _handle ); }
		auto& key() const { return _owner->key( _handle ); }

		// get val (explicit)
		auto& val()       { return operator(); }
		auto& val() const { return operator(); }


		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner->erase( _handle );
			_just_erased = true;
		}

		bool exists() const {
			return _handle.valid();
		}




	// for ITERATOR_BASE:
	private:
		friend Iterator_Base<C,Accessor>;

		inline void _increment() {
			if(_just_erased) {
				_just_erased = false;
				return;
			}
			++_handle.b;
			while(_handle.a < _owner->_buckets.size() && (int)_handle.b == _owner->_buckets[_handle.a].size()) {
				_handle.b = 0;
				++_handle.a;
			}
		}

		inline void _decrement() {
			_just_erased = false;
			if(_handle.b == 0) {
				--_handle.a;
				_handle.b = _owner->_buckets[_handle.a].size() - 1;
			}
		}

		auto _get_comparable() const { return std::make_pair(_handle.a, _handle.b); }

		template<Const_Flag CC>
		auto _will_compare_with(const Accessor<CC>& o) const {
			DCHECK_EQ(_owner, o._owner);
		}




	private:
		Accessor(Const<Hash_Table,C>* owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Hash_Table;


	private:
		Const<Hash_Table,C>* _owner;
		Handle _handle;

		bool _just_erased = false;
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

	public:
		Hash_Table() = default;

		template<class H>
		Hash_Table(H&& hash) : Hash( std::forward<H>(hash) ) {}

		~Hash_Table() {
			if constexpr(!Inplace) {
				for(auto e : *this) {
					auto handle = e.handle();
					_alloc().destruct( _buckets[handle.a][handle.b] );
				}
			}
		}

		Hash_Table(const Hash_Table&) = delete; // todo
		Hash_Table(Hash_Table&&) = default;

		Hash_Table& operator=(const Hash_Table&) = delete; // todo
		Hash_Table& operator=(Hash_Table&&) = default;


		//
		// data
		//
	private:
		Buckets  _buckets;
		int _count = 0;

	private:
		auto& _alloc()       { return *static_cast<      Allocator*>(this); }
		auto& _alloc() const { return *static_cast<const Allocator*>(this); }

		//
		// interface: manipulate element - can be accessed via the Accessor
		//
	public:
		void erase(Handle handle) {
			DCHECK( handle.valid() );
			DCHECK_GT( _count, 0 );
			DCHECK_GT( _buckets.size(), 0 );

			--_count;
			if constexpr(!Inplace) _alloc().destruct( _buckets[handle.a][handle.b] );
			_buckets[handle.a](handle.b).erase();
		}



		auto& key(Handle handle) {
			return _alloc()[ _buckets[handle.a][handle.b] ].key;
		}


		auto& operator[](Handle handle) {
			DCHECK( handle.valid() );
			DCHECK_GT( _count, 0 );
			DCHECK_GT( _buckets.size(), 0 );

			if constexpr(Has_Val) return _kv( _buckets[handle.a][handle.b] ).val;
			else return _kv( _buckets[handle.a][handle.b] ).key;
		}

		auto& operator[](Handle handle) const {
			DCHECK( handle.valid() );
			DCHECK_GT( _count, 0 );
			DCHECK_GT( _buckets.size(), 0 );

			if constexpr(Has_Val) return _alloc()[ _buckets[handle.a][handle.b] ].val;
			else return _alloc()[ _buckets[handle.a][handle.b] ].key;
		}




		auto operator()(Handle handle)       { return Accessor<MUTAB>(this, handle); }
		auto operator()(Handle handle) const { return Accessor<CONST>(this, handle); }


		auto& operator[](const Key& k)       { return operator[]( _find(k) ); }
		auto& operator[](const Key& k) const { return operator[]( _find(k) ); }

		auto operator()(const Key& k)       { return operator()( _find(k) ); }
		auto operator()(const Key& k) const { return operator()( _find(k) ); }


	private:
		Handle _find(const Key& k) const {
			if(_buckets.size() == 0) return Handle();

			auto i_bucket = Hash::operator()(k) % _buckets.size();
			auto& bucket = _buckets[ i_bucket ];

			for(auto e : bucket) {
				if(_kv(e()).key == k) {
					return {i_bucket, e.handle()};
				}
			}

			return Handle();
		}



	public:
		template<class K, class... V>
		auto emplace(K&& k, V&&... v) {

			// re-bucket
			int want_buckets = _count + 1;
			if(_buckets.size()*3/2 < want_buckets) {
				rehash(want_buckets);
			}

			++_count;

			auto i_bucket = Hash::operator()(k) % _buckets.size();

			if constexpr(Inplace) {
				auto b = _buckets[i_bucket].add( k, std::forward<V>(v)... ).handle();
				return Accessor<MUTAB>(this, {i_bucket, b});
			}
			else {
				auto new_element = _alloc().construct( k, std::forward<V>(v)... );
				auto b = _buckets[i_bucket].add( new_element ).handle();
				return Accessor<MUTAB>(this, {i_bucket, b});
			}
		}

		int count() const {
			return _count;
		}

		void rehash(int want_buckets) {
			Buckets new_buckets(want_buckets);

			for(auto bucket : _buckets) {
				for(auto e : bucket()) {
					int i_new_bucket = Hash::operator()( _kv(e()).key ) % want_buckets;
					new_buckets[ i_new_bucket ].add( std::move(e()) );
				}
			}
			_buckets = std::move(new_buckets);
		}

		auto bucket_count() { return _buckets.size(); }

		auto max_bucket_size() const {
			int r = 0;
			for(auto bucket : _buckets) {
				r = std::max(r, bucket().size());
			}
			return r;
		}


	private:
		auto& _kv(Node& node) {
			if constexpr(Inplace) return node;
			else return _alloc()[node];
		}
		auto& _kv(const Node& node) const {
			if constexpr(Inplace) return node;
			else return _alloc()[node];
		}



	public:
		auto begin() {
			Handle h = {0,0};
			while(h.a < _buckets.size() && _buckets[h.a].empty()) ++h.a;
			return Accessor<MUTAB>(this, h);
		}

		auto begin() const {
			Handle h = {0,0};
			while(h.a < _buckets.size() && _buckets[h.a].empty()) ++h.a;
			return Accessor<CONST>(this, h);
		}


		auto end() {
			return Accessor<MUTAB>(this, {_buckets.size(), 0});
		}

		auto end() const {
			return Accessor<CONST>(this, {_buckets.size(), 0});
		}

	};







	struct With_Builder : Hash_Table {
		FORWARDING_CONSTRUCTOR(With_Builder, Hash_Table);

		template<class NEW_HASH>
		using HASH = typename Context<Key, Val, NEW_HASH, Allocator, Inplace> :: With_Builder;

		template<class NEW_ALLOCATOR>
		using ALLOCATOR = typename Context<Key, Val, Hash, NEW_ALLOCATOR, Inplace> :: With_Builder;

		using EXTERNAL = typename Context<Key, Val, Hash, Allocator, false> :: With_Builder;
	};




}; // struct Context
} // namespace Hash_Table
} // namespace internal






template<
	class KEY,
	class VAL = void
>
using Hash_Table = typename internal::hash_table::Context<
	KEY,
	VAL,
	::std::hash<KEY>, // HASH
	salgo::Allocator<int> :: SINGLETON, // ALLOCATOR (int will be rebound anyway)
	std::is_move_constructible_v<KEY> && (std::is_same_v<VAL,void> || std::is_move_constructible_v<VAL>) // INPLACE
> :: With_Builder;











}
