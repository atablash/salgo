#pragma once

namespace salgo {


template<class CRTP>
struct Int_Handle {
	int key = -1;

	Int_Handle() = default;
	Int_Handle(int new_key) : key(new_key) {}

	bool valid() const { return key >= 0; }
	void reset() { key = -1; }

	bool operator==(const CRTP& o) const { return key == o.key; }
	bool operator!=(const CRTP& o) const { return key != o.key; }

	operator int() const { return key; }
};




}


