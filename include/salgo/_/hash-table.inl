#pragma once

#include "hash-table.hpp"

#include "memory-block.inl"
#include "unordered-array.inl"

#include "alloc/array-allocator.inl" // default

#include "hash.hpp"
#include "key-val.hpp"

#include "type-traits.hpp"

#include "helper-macros-on.inc"

namespace salgo::_::hash_table {


// ADD_MEMBER(cached_key);


//
// accessor
//
template<class P, Const_Flag C>
class Accessor : public Accessor_Base<C,Context<P>> {
	using BASE = Accessor_Base<C,Context<P>>;
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Accessor,BASE) {}
	friend Hash_Table<P>;

public:
	// get key
	auto& key()       { return CONT.key( HANDLE ); }
	auto& key() const { return CONT.key( HANDLE ); }

	// get val (explicit)
	auto& val()       { return BASE::operator()(); }
	auto& val() const { return BASE::operator()(); }

	auto& key_val()       { return CONT._kv( CONT._buckets[HANDLE.a][HANDLE.b] ); }
	auto& key_val() const { return CONT._kv( CONT._buckets[HANDLE.a][HANDLE.b] ); }


	void erase() {
		static_assert(C == MUTAB, "called erase() on CONST accessor");
		CONT.erase( HANDLE );
		BASE::_just_erased = true;
	}

	void erase_if_found() {
		if(found()) erase();
	}

	bool found()     const { return HANDLE.valid(); }
	bool not_found() const { return ! found(); }
};



template<class KEY, class VAL, class HASH, class ALLOCATOR, bool INPLACE>
struct Params {
	using Key = KEY;
	using Val = VAL;
	using Hash = HASH;
	using Supplied_Allocator = ALLOCATOR;
	static constexpr bool Inplace = INPLACE;

	static constexpr bool Has_Val = !std::is_same_v<Val, void>;

	using Key_Val = salgo::Key_Val<Key,Val>;

	using Rebound_Allocator = typename Supplied_Allocator ::template VAL<Key_Val>;

	using Node = std::conditional_t<
		Inplace,
		Key_Val,
		typename Rebound_Allocator::Handle_Small
	>;

	using Unordered_Array = typename salgo::Unordered_Array<Node> :: template INPLACE_BUFFER<5>;
	using Buckets = typename salgo::Memory_Block<Unordered_Array> :: DENSE;

	using H0 = typename Buckets::Handle;
	using H1 = typename Unordered_Array::Handle;

	using Handle = hash_table::Handle<Params>;
	using Handle_Small = Handle; // the same currently
};



template<class P> //todo: use pair_handle_base
struct Handle {
	typename P::H0 a;
	typename P::H1 b;

	bool valid() const { return a.valid(); }
};



template<class P, Const_Flag C>
class Reference : public Reference_Base<C,Context<P>>
		// , protected Add_cached_key<Key, Accessor_Caches_Key> // TODO: cache in Inplace_Storage; TODO: add constructed_flag to stack storage
{
	using BASE = Reference_Base<C,Context<P>>;

public:
	using BASE::BASE;

protected:
	bool _just_erased = false;
};



template<class P>
struct End_Iterator {};



//
// iterator
//
template<class P, Const_Flag C>
class Iterator : public Iterator_Base<C,Context<P>>, private P {
	using BASE = Iterator_Base<C,Context<P>>;
	using BASE::BASE;
	// FORWARDING_CONSTRUCTOR(Iterator,BASE) {}
	friend Hash_Table<P>;

	using BASE::_just_erased;

	using typename P::H0;
	using typename P::H1;

private:
	friend Iterator_Base<C,Context<P>>;

	// note:
	// _increment and _decrement are swapped so that elements can be erased externally inside for(auto& e : ht)

	void _decrement() {
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

	void _increment() {
		_just_erased = false;
		--MUT_HANDLE.b;
		while(HANDLE.b == (typename P::Unordered_Array::Handle)(-1)) {
			--MUT_HANDLE.a;
			MUT_HANDLE.b = H1(0);
			if(HANDLE.a >= 0) {
				MUT_HANDLE.b = H1( CONT._buckets[H0(HANDLE.a)].size() - 1);
			}
		}
	}

	auto _get_comparable() const { return std::make_pair(HANDLE.a, HANDLE.b); }

public:
	bool operator!=(End_Iterator<P>) const { return HANDLE.a != -1; }
};







template<class P>
struct Context {
	using Container = Hash_Table<P>;
	using Handle = typename P::Handle;

	template<Const_Flag C>
	using Reference = hash_table::Reference<P,C>;

	template<Const_Flag C>
	using Accessor = hash_table::Accessor<P,C>;

	template<Const_Flag C>
	using Iterator = hash_table::Iterator<P,C>;
};














template<class P>
class Hash_Table : private P::Hash, private P::Rebound_Allocator, protected P {

public:
	using Key = typename P::Key;
	using Val = typename P::Val;

	// `using typename P::Handle;` doesn't work with gcc 7.3.0 - produces ambiguities
	using Handle       = typename P::Handle;
	using Handle_Small = typename P::Handle_Small;


private:
	friend Accessor<P,MUTAB>;
	friend Accessor<P,CONST>;
	friend Iterator<P,MUTAB>;
	friend Iterator<P,CONST>;

	using typename P::Key_Val;
	using typename P::Buckets;
	using typename P::Node;
	using typename P::Rebound_Allocator;

	using typename P::H0;
	using typename P::H1;

public:
	Hash_Table() = default;

	// template<class H>
	// Hash_Table(H&& hash) : Hash( std::forward<H>(hash) ) {}

	// trivially templating this function makes both gcc and clang inteligently select between initializer_list and variadic template constructor
	template<class KV = Key_Val> // REQUIRES( !std::is_move_constructible_v<KV> )
	Hash_Table(std::initializer_list<KV> il) {
		for(auto& e : il) emplace( e );
	}

	// variadic template constructor - an alternative to initializer_list that works with move-only types
	template<class... LIST,
		class = std::enable_if_t< is_constructible_from_all<Key_Val, LIST...>::value >
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
		if constexpr(!P::Inplace) {
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
	auto& _alloc()       { return *static_cast<      Rebound_Allocator*>(this); }
	auto& _alloc() const { return *static_cast<const Rebound_Allocator*>(this); }

	//
	// interface: manipulate element - can be accessed via the Accessor
	//
public:
	void erase(Handle handle) {
		DCHECK( handle.valid() );
		DCHECK_GT( _count, 0 );
		DCHECK_GT( _buckets.size(), 0 );

		--_count;
		if constexpr(!P::Inplace) _alloc().destruct( _buckets[handle.a][handle.b] );
		_buckets[handle.a](handle.b).erase();
	}



	auto& key(Handle handle) {
		return _kv( _buckets[handle.a][handle.b] ).key;
	}


	auto& operator[](Handle handle) {
		DCHECK( handle.valid() );
		DCHECK_GT( _count, 0 );
		DCHECK_GT( _buckets.size(), 0 );

		if constexpr(P::Has_Val) return _kv( _buckets[handle.a][handle.b] ).val;
		else return _kv( _buckets[handle.a][handle.b] ).key;
	}

	auto& operator[](Handle handle) const {
		DCHECK( handle.valid() );
		DCHECK_GT( _count, 0 );
		DCHECK_GT( _buckets.size(), 0 );

		if constexpr(P::Has_Val) return _kv( _buckets[handle.a][handle.b] ).val;
		else return _kv( _buckets[handle.a][handle.b] ).key;
	}




	auto operator()(Handle handle)       { return Accessor<P,MUTAB>(this, handle); }
	auto operator()(Handle handle) const { return Accessor<P,CONST>(this, handle); }


	auto& operator[](Any_Tag)       { DCHECK(not_empty()) << "hash_table[ANY] called on empty hash table"; return begin().accessor().data(); }
	auto& operator[](Any_Tag) const { DCHECK(not_empty()) << "hash_table[ANY] called on empty hash table"; return begin().accessor().data(); }

	auto operator()(Any_Tag)       { return begin().accessor(); }
	auto operator()(Any_Tag) const { return begin().accessor(); }


	auto& operator[](const Key& k)       { return operator[]( _find(k) ); }
	auto& operator[](const Key& k) const { return operator[]( _find(k) ); }

	// // get accessor by Key, caching Key if possible
	// auto operator()(const Key& k)       { auto handle = _find(k); auto a = Accessor<MUTAB>(this, handle); if constexpr(Accessor_Caches_Key) a.cached_key = k; return a; }
	// auto operator()(const Key& k) const { auto handle = _find(k); auto a = Accessor<CONST>(this, handle); if constexpr(Accessor_Caches_Key) a.cached_key = k; return a; }

	auto operator()(const Key& k)       { return Accessor<P,MUTAB>(this, _find(k)); }
	auto operator()(const Key& k) const { return Accessor<P,CONST>(this, _find(k)); }


private:
	Handle _find(const Key& k) const {
		if(_buckets.size() == 0) return Handle();

		int i_bucket = P::Hash::operator()(k) % _buckets.size();
		auto& bucket = _buckets[ i_bucket ];

		for(auto& e : bucket) {
			if(_kv(e()).key == k) {
				return {(H0)i_bucket, e.handle()};
			}
		}

		return Handle();
	}



public: // todo: remove duplicates, add REQUIRES(kv_like) - similar implementation in kd-tree
	auto emplace(      Key_Val&  kv) { return emplace_kv( kv ); }
	auto emplace(const Key_Val&  kv) { return emplace_kv( kv ); }
	auto emplace(      Key_Val&& kv) { return emplace_kv( std::move(kv) ); }
	auto emplace(const Key_Val&& kv) { return emplace_kv( std::move(kv) ); }

public: // todo: remove duplicates, add REQUIRES(kv_like) - similar implementation in kd-tree
	auto emplace_if_not_found(      Key_Val&  kv) { return emplace_kv_inf( kv ); }
	auto emplace_if_not_found(const Key_Val&  kv) { return emplace_kv_inf( kv ); }
	auto emplace_if_not_found(      Key_Val&& kv) { return emplace_kv_inf( std::move(kv) ); }
	auto emplace_if_not_found(const Key_Val&& kv) { return emplace_kv_inf( std::move(kv) ); }

private:
	template<class KV>
	auto emplace_kv(KV&& kv) {
		if constexpr(P::Has_Val) {
			return emplace( std::forward<KV>(kv).key, std::forward<KV>(kv).val );
		}
		else {
			return emplace( std::forward<KV>(kv).key );
		}
	}

	template<class KV>
	auto emplace_kv_inf(KV&& kv) {
		auto r = operator()( kv.key );
		if(r.found()) return r;

		if constexpr(P::Has_Val) {
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
	auto emplace_if_not_found(K&& k, V&&... v) {
		auto r = operator()( k );
		if(r.found()) return r;
		else return emplace( std::forward<K>(k), std::forward<V>(v)... );
	}

	template<class K, class... V>
	auto emplace(K&& k, V&&... v) {

		// re-bucket
		int want_buckets = _want_buckets_for_count( _count );
		if(_buckets.size()*3/2 < want_buckets) {
			rehash(want_buckets);
		}

		++_count;

		auto i_bucket = typename P::H0(
			P::Hash::operator()(k) % _buckets.size()
		);

		if constexpr(P::Inplace) {
			auto b = _buckets[i_bucket].add( std::forward<K>(k), std::forward<V>(v)... ).handle();
			return Accessor<P,MUTAB>(this, Handle{i_bucket, b});
		}
		else {
			auto new_element = _alloc().construct( std::forward<K>(k), std::forward<V>(v)... );
			auto b = _buckets[i_bucket].add( new_element ).handle();
			return Accessor<P,MUTAB>(this, Handle{i_bucket, b});
		}
	}

	int count() const {
		return _count;
	}

	bool is_empty() const {
		return _count == 0;
	}

	bool not_empty() const {
		return !is_empty();
	}

	void rehash(int want_buckets) {
		Buckets new_buckets(want_buckets);

		for(auto& bucket : _buckets) {
			for(auto& e : bucket()) {
				int i_new_bucket = P::Hash::operator()( _kv(e()).key ) % want_buckets;
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
		if constexpr(P::Inplace) return node;
		else return _alloc()[node];
	}
	auto& _kv(const Node& node) const {
		if constexpr(P::Inplace) return node;
		else return _alloc()[node];
	}



public:
	auto begin() {
		Handle h = { H0( _buckets.size() ), H1(0) };
		return Iterator<P,MUTAB>(this, h).next();
	}

	auto begin() const {
		Handle h = {(H0)_buckets.size(), H1(0)};
		return Iterator<P,CONST>(this, h).next();
	}


	auto end() const { return End_Iterator<P>(); }

};






template<class P>
class With_Builder : public Hash_Table<P> {
	using BASE = Hash_Table<P>;

	using typename P::Key;
	using typename P::Val;
	using typename P::Hash;
	using typename P::Supplied_Allocator;
	using P::Inplace;

public:
	using BASE::BASE;

	template<class NEW_HASH>
	using HASH = With_Builder< Params<Key, Val, NEW_HASH, Supplied_Allocator, Inplace>>;

	template<class NEW_ALLOCATOR>
	using ALLOCATOR = With_Builder< Params<Key, Val, Hash, NEW_ALLOCATOR, Inplace>>;

	using EXTERNAL = With_Builder< Params<Key, Val, Hash, Supplied_Allocator, false>>;
};

} // namespace salgo::_::hash_table

#include "helper-macros-off.inc"







//
// tuple interface for key_vals
//
#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-tags"
#endif
template<class CTX, salgo::Const_Flag CF>
struct std::tuple_size<salgo::_::hash_table::Accessor<CTX, CF>> : std::tuple_size<typename CTX::Key_Val> {};

template<std::size_t ITH, class CTX, salgo::Const_Flag CF>
struct std::tuple_element<ITH, salgo::_::hash_table::Accessor<CTX, CF>> : std::tuple_element<ITH, typename CTX::Key_Val> {};
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace std {
	template<std::size_t ITH, class CTX, salgo::Const_Flag CF>
	constexpr auto& get(       salgo::_::hash_table::Accessor<CTX, CF>& acc ) noexcept { return get<ITH>(acc.key_val()); }

	template<std::size_t ITH, class CTX, salgo::Const_Flag CF>
	constexpr auto& get( const salgo::_::hash_table::Accessor<CTX, CF>& acc ) noexcept { return get<ITH>(acc.key_val()); }
}
