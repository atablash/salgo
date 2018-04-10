#pragma once

namespace salgo {


template<class INT, class CRTP>
struct Int_Handle {
	INT key = -1;

	Int_Handle() = default;
	Int_Handle(INT new_key) : key(new_key) {}

	bool valid() const { return key >= 0; }
	void reset() { key = -1; }

	bool operator==(const CRTP& o) const { return key == o.key; }
	bool operator!=(const CRTP& o) const { return key != o.key; }

	operator INT() const { return key; }
	//operator long() const { return key; }

	//operator bool() const { return valid(); }
};




}


