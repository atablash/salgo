#pragma once

namespace salgo {









template<class CRTP, class INT=int, INT default_value = std::numeric_limits<INT>::max()>
struct Int_Handle_Base {
	INT a = default_value;

	Int_Handle_Base() = default;
	Int_Handle_Base(INT new_key) : a(new_key) {}

	bool valid() const { return a != default_value; }
	void reset() { a = default_value; }

	bool operator==(const CRTP& o) const { return a == o.a; }
	bool operator!=(const CRTP& o) const { return a != o.a; }

	operator INT() const { return a; }

	CRTP& operator++()    { ++a; return _self(); }
	CRTP  operator++(int) { auto old = _self(); ++(*this); return old; }

	CRTP& operator--()    { --a; return _self(); }
	CRTP  operator--(int) { auto old = _self(); --(*this); return old; }


	static_assert(std::is_integral_v<INT>);


	CRTP_COMMON(Int_Handle_Base, CRTP)
};





//
// default CRTP instantiation
//
template<class INT=int, INT default_value = std::numeric_limits<INT>::max()>
struct Int_Handle : Int_Handle_Base<Int_Handle<INT,default_value>, INT, default_value> {
	using BASE = Int_Handle_Base<Int_Handle, INT, default_value>;
	FORWARDING_CONSTRUCTOR(Int_Handle, BASE) {}
};










template<
	class CRTP,
	class HANDLE_A,
	class HANDLE_B
>
struct Pair_Handle_Base {

	HANDLE_A a = HANDLE_A();
	HANDLE_B b = HANDLE_B();

	Pair_Handle_Base() = default;

	Pair_Handle_Base(HANDLE_A aa, HANDLE_B bb) : a(aa), b(bb) {}

	bool valid() const { return a.valid(); }
	void reset() { a.reset(); }

	bool operator==(const Pair_Handle_Base& o) const { return a==o.a && b==o.b; }
	bool operator!=(const Pair_Handle_Base& o) const { return !(*this == o); }
};







}


