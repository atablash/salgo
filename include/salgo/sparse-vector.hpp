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

	std::vector<Node> v;
	int num_existing = 0;

public:
	Sparse_Vector() = default;
	Sparse_Vector(int size) : v(size), num_existing(size) {}

public:
	inline VAL& operator[](int key) {
		_check_key(key);
		return v[key].val;
	}

	inline const VAL& operator[](int key) const {
		_check_key(key);
		return v[key].val;
	}

	inline void erase(int key) {
		_check_key(key);
		DCHECK(v[key].exists) << "erasing already erased element";
		v[key].exists = false;
		v[key].val.destruct();
		--num_existing;
	}

	inline bool exists(int key) const {
		DCHECK_GE(key, 0) << "index out of bounds";
		DCHECK_LT(key, (int)v.size()) << "index out of bounds";
		return v[key].exists;
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
	private:
		Iterator(Node* new_node, Node* new_end_node)
				: node(new_node), end_node(new_end_node) {
			if(node != end_node && !node->exists) _increment();
		}

		friend Sparse_Vector;


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
		bool operator==(const Iterator& o) const {
			DCHECK_EQ(end_node, o.end_node);
			return node == o.node;
		}

		bool operator!=(const Iterator& o) const {
			return !(node == o.node);
		}




	public:
		VAL& operator*() {
			return node->val;
		}

		const VAL& operator*() const {
			return node->val;
		}


		VAL* operator->() {
			return &node->val;
		}

		const VAL* operator->() const {
			return &node->val;
		}





	private:
		inline void _increment() {
			do {
				++node;
			} while(node != end_node && !node->exists);
		}

		inline void _decrement() {
			do {
				--node;
			} while(!node->exists);
		}

	private:
		Const<Node,C>* node;
		Const<Node,C>* const end_node;
	};

public:
	inline auto begin() {
		return Iterator<MUTAB>(&v[0], &v[v.size()]);
	}

	inline auto begin() const {
		return Iterator<CONST>(&v[0], &v[v.size()]);
	}


	inline auto end() {
		return Iterator<MUTAB>( &v[v.size()], &v[v.size()] );
	}

	inline auto end() const {
		return Iterator<CONST>( &v[v.size()], &v[v.size()] );
	}



private:
	inline void _check_key(int key) const {
		DCHECK_GE(key, 0) << "index out of bounds";
		DCHECK_LT(key, (int)v.size()) << "index out of bounds";
		DCHECK( v[key].exists ) << "accessing erased element";
	}
};






}
