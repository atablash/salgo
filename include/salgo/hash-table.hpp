#pragma once

#include "memory-block.hpp"
#include "unordered-vector.hpp"
#include "vector-allocator.hpp"
#include "hash.hpp"
#include "key-val.hpp"

#include "helper-macros-on"

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
		typename Allocator::Handle_Small
	>;

	using List = typename salgo::Unordered_Vector<Node> :: template INPLACE_BUFFER<5>;
	using Buckets = typename salgo::Memory_Block<List> :: DENSE;



	struct Handle {
		typename Buckets::Index a;
		typename List::Handle b;

		bool valid() const { return a.valid(); }
	};

	using Handle_Small = Handle; // the same currently










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

		using BASE::_just_erased;

	public:
		// get key
		auto& key()       { return CONT.key( HANDLE ); }
		auto& key() const { return CONT.key( HANDLE ); }

		// get val (explicit)
		auto& val()       { return BASE::operator()(); }
		auto& val() const { return BASE::operator()(); }


		void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			CONT.erase( HANDLE );
			_just_erased = true;
		}

		bool exists() const {
			return HANDLE.valid();
		}
	};



	struct End_Iterator {};

	//
	// iterator
	//
	template<Const_Flag C>
	class Iterator : public Iterator_Base<C,Context> {
		using BASE = Iterator_Base<C,Context>;
		FORWARDING_CONSTRUCTOR(Iterator,BASE) {}
		friend Hash_Table;

		using BASE::_just_erased;

	private:
		friend Iterator_Base<C,Context>;

		// note:
		// _increment and _decrement are swapped so that elements can be erased externally inside for(auto& e : ht)

		inline void _decrement() {
			if(_just_erased) {
				_just_erased = false;
				return;
			}
			++HANDLE.b;
			while(HANDLE.a < CONT._buckets.size() && (int)HANDLE.b == CONT._buckets[HANDLE.a].size()) {
				HANDLE.b = 0;
				++HANDLE.a;
			}
		}

		inline void _increment() {
			_just_erased = false;
			--HANDLE.b;
			while(HANDLE.b == (typename List::Handle)(-1)) {
				--HANDLE.a;
				HANDLE.b = 0;
				if(HANDLE.a >= 0) HANDLE.b = CONT._buckets[HANDLE.a].size() - 1;
			}
		}

		auto _get_comparable() const { return std::make_pair(HANDLE.a, HANDLE.b); }

	public:
		bool operator!=(End_Iterator) const { return HANDLE.a >= 0; }
	};















	class Hash_Table : private Hash, private Allocator {

	public:
		using Key = Context::Key;
		using Val = Context::Val;

		using Handle = Context::Handle;
		using Handle_Small = Context::Handle_Small;


	private:
		friend Accessor<MUTAB>;
		friend Accessor<CONST>;
		friend Iterator<MUTAB>;
		friend Iterator<CONST>;

	public:
		Hash_Table() = default;

		// template<class H>
		// Hash_Table(H&& hash) : Hash( std::forward<H>(hash) ) {}

		template<class KV = Key_Val, REQUIRES(std::is_move_constructible_v<KV>)>
		Hash_Table(std::initializer_list<KV> il) {
			for(auto& e : il) emplace( e );
		}

		template<class... LIST,
			REQUIRES( is_constructible<Key_Val, LIST...>::value )
		>
		Hash_Table(LIST&&... list) : _buckets( sizeof...(list) ) {
			_list_init( std::forward<LIST>(list)... );
		}

	private:
		template<class EL, class... LIST>
		void _list_init(EL&& el, LIST&&... list) {
			emplace( std::forward<EL>(el) );
			_list_init(std::forward<LIST>(list)...);
		}
		void _list_init() {}

	public:
		~Hash_Table() {
			if constexpr(!Inplace) {
				for(auto e : *this) {
					auto handle = e.handle();
					_alloc()( _buckets[handle.a][handle.b] ).destruct();
				}
			}
		}

		Hash_Table(const Hash_Table&) = default;
		Hash_Table(Hash_Table&&) = default;

		Hash_Table& operator=(const Hash_Table&) = default;
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

			int i_bucket = Hash::operator()(k) % _buckets.size();
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
		void reserve(int want_elements) {
			int want_buckets = _want_buckets_for_count( want_elements );
			rehash(want_buckets);
		}
	
	private:
		int _want_buckets_for_count(int count) {
			return count + 1;
		}

	public:
		template<class K, class... V>
		auto emplace(K&& k, V&&... v) {

			// re-bucket
			int want_buckets = _want_buckets_for_count( _count );
			if(_buckets.size()*3/2 < want_buckets) {
				rehash(want_buckets);
			}

			++_count;

			int i_bucket = Hash::operator()(k) % _buckets.size();

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

		bool empty() const {
			return _count == 0;
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
			Handle h = {_buckets.size(), 0};
			return Iterator<MUTAB>(this, h).next();
		}

		auto begin() const {
			Handle h = {_buckets.size(), 0};
			return Iterator<CONST>(this, h).next();
		}


		auto end() const { return End_Iterator(); }

	};







	struct With_Builder : Hash_Table {
		using Hash_Table::Hash_Table;

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
	salgo::Vector_Allocator<int>, // ALLOCATOR (int will be rebound anyway), used only when not INPLACE
	std::is_move_constructible_v<KEY> && (std::is_same_v<VAL,void> || std::is_move_constructible_v<VAL>) // INPLACE
> :: With_Builder;











}

#include "helper-macros-off"
