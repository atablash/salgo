#include <salgo/all>

#include "../../common.hpp" // competitive programming macros, etc.

using namespace std;
using namespace salgo::discr;



int main() {

	// read N, K, P from stdin
	const int INPUT_N   = RI;
	const int INPUT_K   = RI;
	const int INPUT_MOD = RI;


	// we'll operate on integers modulo INPUT_MOD
	using Z = Modulo;
	auto modulo_context = Modulo_Context( INPUT_MOD );



	const int MAX_N = 1009;
	const int SPLIT = 11;

	Z f[ MAX_N ][ SPLIT ];
	Z g[ MAX_N ][ SPLIT ];
	Z h[ MAX_N ][ SPLIT ];

	auto get_h = [&](int n, int k) {
		if(k >= SPLIT) return h[n][SPLIT-1];
		return h[n][k];
	};


	// preprocess binomials table
	Z binomial[ MAX_N ][ MAX_N ];
	FOR(n, MAX_N) {
		binomial[n][0] = 1;
		FO(k, 1, n+1) {
			binomial[n][k] =
				  binomial[n-1][k-1]
				+ binomial[n-1][k  ];
		}
	}


	// compute answers for all N,K
	FO(k, 0, SPLIT) f[0][k] = 1;
	FO(k, 0, SPLIT) g[0][k] = 1;
	FO(n, 1, MAX_N) {
		FO(k, 0, SPLIT) {
			FOR(n1, n) {
				int n2 = n-1 - n1;

				if(k-1 >= 0) f[n][k] += binomial[n-1][n1] * (
						  f[n1][k  ] * f[n2][k-1]
						+ f[n1][k-1] * f[n2][k  ]
						- f[n1][k-1] * f[n2][k-1]
				);

				if(k-1 >= 0) g[n][k] += binomial[n-1][n1] * (
					f[n1][k-1] * g[n2][k]
				);

				h[n][k] += binomial[n-1][n1] * (
					g[n1][k] * g[n2][k]
				);
			}
		}
	}


	auto solve = [&](int n, int k) {
		return get_h(n,k) - get_h(n, k-1);
	};

	cout << solve(INPUT_N, INPUT_K) << endl;

	return 0;
}

