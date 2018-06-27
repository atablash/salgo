#pragma once

#include "memory-block.hpp"
#include "unordered-vector.hpp"
#include "crude-allocator.hpp"
#include "random-allocator.hpp"
#include "hash.hpp"
#include "key-val.hpp"


namespace salgo {






namespace internal {
namespace hash_table {






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

	using Key_Val = salgo::Key_Val<Key,Val>;

	using Allocator = typename _ALLOCATOR :: template VAL<Key_Val>;


	using Node = std::conditional_t<
		Inplace,
		Key_Val,
		typename Allocator::Small_Handle
	>;

	using List = typename salgo::Unordered_Vector<Node> :: template INPLACE_BUFFER<5>;
	using Buckets = typename salgo::Memory_Block<List> :: DENSE;



	struct Handle {
		typename Buckets::Index a;
		typename List::Handle b;

		bool valid() const { return a.valid(); }
	};

	using Small_Handle = Handle; // the same currently










	//
	// forward declarations
	//
	class Hash_Table;
	using Container = Hash_Table;





	template<Const_Flag C>
	class Reference : public Reference_Base<C,Context> {
		using BASE = Reference_Base<C,Context>;

	public:
		FORWARDING_CONSTRUCTOR(Reference, BASE) {}

	protected:
		bool _just_erased = false;
	};



	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor : public Accessor_Base<C,Context> {
		using BASE = Accessor_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Accessor,BASE) {}
		friend Hash_Table;

		using BASE::_container;
		using BASE::_handle;
		using BASE::_just_erased;

	public:
		// get key
		auto& key()       { return _container().key( _handle() ); }
		auto& key() const { return _container().key( _handle() ); }

		// get val (explicit)
		auto& val()       { return BASE::operator()(); }
		auto& val() const { return BASE::operator()(); }


		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_container().erase( _handle() );
			_just_erased = true;
		}

		bool exists() const {
			return _handle().valid();
		}
	};




	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Iterator,BASE) {}
		friend Hash_Table;

		using BASE::_container;
		using BASE::_handle;
		using BASE::_just_erased;

	private:
		friend Iterator_Base<C,Context>;

		inline void _increment() {
			if(_just_erased) {
				_just_erased = false;
				return;
			}
			++_handle().b;
			while(_handle().a < _container()._buckets.size() && (int)_handle().b == _container()._buckets[_handle().a].size()) {
				_handle().b = 0;
				++_handle().a;
			}
		}

		inline void _decrement() {
			_just_erased = false;
			if(_handle().b == 0) {
				--_handle().a;
				_handle().b = _container()._buckets[_handle().a].size() - 1;
			}
		}

		auto _get_comparable() const { return std::make_pair(_handle().a, _handle().b); }
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

		Hash_Table(std::initializer_list<Key_Val>&& il) {
			for(auto&& e : il) emplace( std::move(e) );
		}

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
			return _kv( _buckets[handle.a][handle.b] ).key;
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

			if constexpr(Has_Val) return _kv( _buckets[handle.a][handle.b] ).val;
			else return _kv( _buckets[handle.a][handle.b] ).key;
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

			for(auto& e : bucket) {
				if(_kv(e()).key == k) {
					return {i_bucket, e.handle()};
				}
			}

			return Handle();
		}



	public:
		auto emplace(      Key_Val&  kv) { return emplace_kv( kv ); }
		auto emplace(const Key_Val&  kv) { return emplace_kv( kv ); }
		auto emplace(      Key_Val&& kv) { return emplace_kv( std::move(kv) ); }
		auto emplace(const Key_Val&& kv) { return emplace_kv( std::move(kv) ); }

	private:
		template<class KV>
		auto emplace_kv(KV&& kv) {
			if constexpr(Has_Val) {
				return emplace( std::forward<KV>(kv).key, std::forward<KV>(kv).val );
			}
			else {
				return emplace( std::forward<KV>(kv).key );
			}
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
				auto b = _buckets[i_bucket].add( std::forward<K>(k), std::forward<V>(v)... ).handle();
				return Accessor<MUTAB>(this, Handle{i_bucket, b});
			}
			else {
				auto new_element = _alloc().construct( std::forward<K>(k), std::forward<V>(v)... );
				auto b = _buckets[i_bucket].add( new_element ).handle();
				return Accessor<MUTAB>(this, Handle{i_bucket, b});
			}
		}

		int count() const {
			return _count;
		}

		void rehash(int want_buckets) {
			Buckets new_buckets(want_buckets);

			for(auto& bucket : _buckets) {
				for(auto& e : bucket()) {
					int i_new_bucket = Hash::operator()( _kv(e()).key ) % want_buckets;
					new_buckets[ i_new_bucket ].add( std::move(e()) );
				}
			}
			_buckets = std::move(new_buckets);
		}

		auto bucket_count() { return _buckets.size(); }

		auto max_bucket_size() const {
			int r = 0;
			for(auto& bucket : _buckets) {
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
			return Accessor<MUTAB>(this, h).iterator();
		}

		auto begin() const {
			Handle h = {0,0};
			while(h.a < _buckets.size() && _buckets[h.a].empty()) ++h.a;
			return Accessor<CONST>(this, h).iterator();
		}


		auto end() {
			return Accessor<MUTAB>(this, Handle{_buckets.size(), 0}).iterator();
		}

		auto end() const {
			return Accessor<CONST>(this, Handle{_buckets.size(), 0}).iterator();
		}

	};







	struct With_Builder : Hash_Table {
		FORWARDING_CONSTRUCTOR(With_Builder, Hash_Table) {}

		With_Builder(std::initializer_list<Key_Val>&& il) : Hash_Table( std::move(il) ) {}

		//FORWARDING_INITIALIZER_LIST_CONSTRUCTOR(With_Builder, Hash_Table) {}

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
	Hash<KEY>, // HASH
	salgo::Random_Allocator<int> :: SINGLETON, // ALLOCATOR (int will be rebound anyway), used only when not INPLACE
	std::is_move_constructible_v<KEY> && (std::is_same_v<VAL,void> || std::is_move_constructible_v<VAL>) // INPLACE
> :: With_Builder;











}
