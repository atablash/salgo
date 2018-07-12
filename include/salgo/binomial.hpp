#pragma once


namespace salgo {




template<class VAL>
class Binomial {
private:
	int n = 0;
	int k = 0;
	VAL val = 1;

public:
	Binomial() = default;

	VAL operator()(int nn, int kk) {
		if(outside(nn,kk)) return 0;

		while(k > kk) move_l();
		while(n > nn) move_u();
		while(n < nn) move_d();
		while(k < kk) move_r();

		return val;
	}

private:
	static bool outside(int nn, int kk) {
		return nn < 0 || kk < 0 || kk > nn;
	}

private:
	void move_l() {
		--k;
		val *= k+1;
		val /= n-k;
	}

	void move_r() {
		++k;
		val *= n-k+1;
		val /= k;
	}

	void move_d() {
		++n;
		val *= n;
		val /= n-k;
	}

	void move_u() {
		--n;
		val *= n-k+1;
		val /= n+1;
	}
};




} // namespace salgo


