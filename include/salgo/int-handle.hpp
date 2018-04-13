#pragma once

namespace salgo {


template<class CRTP, class INT=int>
struct Int_Handle {
	INT key = std::numeric_limits<INT>::max();

	Int_Handle() = default;
	Int_Handle(INT new_key) : key(new_key) {}

	bool valid() const { return key != std::numeric_limits<INT>::max(); }
	void reset() { key = std::numeric_limits<INT>::max(); }

	bool operator==(const CRTP& o) const { return key == o.key; }
	bool operator!=(const CRTP& o) const { return key != o.key; }

	operator INT() const { return key; }

	CRTP& operator++()    { ++key; return _self(); }
	CRTP  operator++(int) { auto old = _self(); ++(*this); return old; }

	CRTP& operator--()    { --key; return _self(); }
	CRTP  operator--(int) { auto old = _self(); --(*this); return old; }


	static_assert(std::is_integral_v<INT>);


CRTP_COMMON(Int_Handle, CRTP)
};




}


