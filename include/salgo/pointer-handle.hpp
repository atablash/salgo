#pragma once

namespace salgo {




template<class PTR, class CRTP>
struct Pointer_Handle {
	PTR pointer = nullptr;

	Pointer_Handle() = default;
	Pointer_Handle(PTR new_pointer) : pointer(new_pointer) {}

	bool valid() const { return pointer != nullptr; }
	void reset() { pointer = nullptr; }

	bool operator==(const CRTP& o) const { return pointer == o.pointer; }
	bool operator!=(const CRTP& o) const { return pointer != o.pointer; }

	operator PTR() const { return pointer; }
};




}