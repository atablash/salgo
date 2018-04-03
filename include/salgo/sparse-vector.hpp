#pragma once

#include "const-flag.hpp"
#include "stack-storage.hpp"


namespace salgo {







template<class VAL>
class Sparse_Vector {
private:
	struct Node {

		template<class... ARGS>
		Node(ARGS&&... args) : val( std::forward<ARGS>(args)... ), exists(true) {}

		~Node() {
			if(exists) {
				val.destruct();
			}
		}

		Stack_Storage<VAL> val;
		bool exists;
	};





	//
	// data
	//
private:
	std::vector<Node> v;
	int num_existing = 0;





	//
	// construction
	//
public:
	Sparse_Vector() = default;
	Sparse_Vector(int size) : v(size), num_existing(size) {}






	//
	// interface: manipulate element - can be accessed via the Accessor
	//
public:
	inline void erase(int key) {
		_check_bounds(key);
		DCHECK( v[key].exists ) << "erasing already erased element";
		v[key].exists = false;
		v[key].val.destruct();
		--num_existing;
	}

	inline bool exists(int key) const {
		_check_bounds(key);
		return v[ key ].exists;
	}

	inline VAL& at(int key) {
		_check_bounds(key);
		return v[key].val;
	}

	inline const VAL& at(int key) const {
		_check_bounds(key);
		return v[key].val;
	}






private:
	inline void _check_bounds(int key) const {
		DCHECK_GE( key, 0 ) << "index out of bounds";
		DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
	}








	//
	// accessor
	//
public:
	template<Const_Flag C>
	class Accessor {
	public:
		inline int key() const {
			return _key;
		}

		inline Const<VAL,C>& val() {
			DCHECK( _owner.v[ _key ].exists ) << "accessing erased element";
			return _owner.at( _key );
		}

		inline const VAL& val() const {
			DCHECK( _owner.v[ _key ].exists ) << "accessing erased element";
			return _owner.at( _key );
		}

		inline void erase() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.erase( _key );
		}

		inline bool exists() const {
			return _owner.exists( _key );
		}


	private:
		Accessor(Const<Sparse_Vector,C>& owner, int key)
			: _owner(owner), _key(key) {}

		friend Sparse_Vector;


	private:
		Const<Sparse_Vector,C>& _owner;
		const int _key;
	};







	//
	// interface
	//
public:
	inline auto operator[](int key) {
		DCHECK_GE( key, 0 ) << "index out of bounds";
		DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
		return Accessor<MUTAB>(*this, key);
	}

	inline auto operator[](int key) const {
		DCHECK_GE( key, 0 ) << "index out of bounds";
		DCHECK_LT( key, (int)v.size() ) << "index out of bounds";
		return Accessor<CONST>(*this, key);
	}

	template<class... ARGS>
	inline void emplace_back(ARGS&&... args) {
		v.emplace_back( std::forward<ARGS>(args)... );
		++num_existing;
	}


	inline int count() const {
		return num_existing;
	}


	static inline constexpr int domain_begin() {
		return 0;
	}

	inline int domain_end() const {
		return v.size();
	}


	//
	// FUN is (int old_key, int new_key) -> void
	//
	template<class FUN>
	void compact(const FUN& fun = [](int,int){}) {
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






private:
	template<Const_Flag C>
	class Iterator {

	public:
		inline auto& operator++() {
			_increment();
			return *this; }

		inline auto operator++(int) {
			auto old = *this;
			_increment();
			return old; }

		inline auto& operator--() {
			_decrement();
			return *this; }

		inline auto operator--(int) {
			auto old = *this;
			_decrement();
			return old; }


	public:
		template<Const_Flag CC>
		inline bool operator==(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
			return _key == o._key;
		}

		template<Const_Flag CC>
		inline bool operator!=(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
			return _key != o._key;
		}

		template<Const_Flag CC>
		inline bool operator<(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
			return _key < o._key;
		}

		template<Const_Flag CC>
		inline bool operator>(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
			return _key > o._key;
		}

		template<Const_Flag CC>
		inline bool operator<=(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
			return _key <= o._key;
		}

		template<Const_Flag CC>
		inline bool operator>=(const Iterator<CC>& o) const {
			DCHECK_EQ(&_owner, &o._owner);
			return _key >= o._key;
		}




	public:
		inline auto operator*() const {  return Accessor<C>(_owner, _key);  }

		// unable to implement if using accessors:
		// auto operator->()       {  return &container[idx];  }




	private:
		inline void _increment() {
			do {
				++_key;
			} while(_key != _owner.domain_end() && !_owner.v[ _key ].exists);
		}

		inline void _decrement() {
			do {
				--_key;
			} while(!_owner.v[ _key ].exists);
		}




	private:
		inline Iterator(Const<Sparse_Vector,C>& owner, int key)
				: _owner(owner), _key(key) {
			if(key != owner.domain_end() && !owner.v[key].exists) _increment();
		}

		friend Sparse_Vector;

	private:
		Const<Sparse_Vector,C>& _owner;
		int _key;
	};





public:
	inline auto begin() {
		return Iterator<MUTAB>(*this, 0);
	}

	inline auto begin() const {
		return Iterator<CONST>(*this, 0);
	}

	inline auto cbegin() const {
		return Iterator<CONST>(*this, 0);
	}


	inline auto end() {
		return Iterator<MUTAB>(*this, v.size());
	}

	inline auto end() const {
		return Iterator<CONST>(*this, v.size());
	}

	inline auto cend() const {
		return Iterator<CONST>(*this, v.size());
	}

};






}
