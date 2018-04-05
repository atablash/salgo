#pragma once

namespace salgo {




template<class T, class CRTP>
struct Pointer_Handle {
	T* pointer = nullptr;

	Pointer_Handle() = default;
	Pointer_Handle(T* new_pointer) : pointer(new_pointer) {}

	bool valid() const { return pointer != nullptr; }
	void reset() { pointer = nullptr; }

	bool operator==(const CRTP& o) const { return pointer == o.pointer; }
	bool operator!=(const CRTP& o) const { return pointer != o.pointer; }

	operator T*() const { return pointer; }
};




}