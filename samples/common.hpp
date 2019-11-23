#pragma once


/////////
//
// These are not part of Salgo, but make prototyping faster:
//
#if USE_FAST_IO
	#include "fast-io.hpp"
	#define cin fast_cin
	#define cout fast_cout
	#define endl fast_endl
#else
	#include "slow-io.hpp"
	#define cin std::cin
	#define cout std::cout
	#define endl '\n'
#endif









/////////
//
// These are not part of Salgo, but make prototyping faster:
//

#define UI unsigned int

#define LL long long
#define ULL unsigned long long

#define PII pair<int,int>


#define RI ri()
#define RUI rui()

#define RLL rll()
#define RULL rull()

#define RSTR rstr()



#define FOR(i,n) for(int i=0; i<int(n); ++i)
#define FO(i,a,b) for(int i=(a); i<int(b); ++i)
#define OF(i,a,b) for(int i=(b)-1; i>=int(a); --i)



#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((b)<(a) ? (a) : (b))

#define REMIN(a,b) ((a) = min(a,b))
#define REMAX(a,b) ((a) = max(a,b))

#define ALL(c) (c).begin(),(c).end()






/////////
//
// These are not part of Salgo, but make prototyping faster:
//
int ri()   { int r; cin >> r; return r; }
UI  rui()  { UI r; cin >> r; return r; }

LL  rll()  { LL r;  cin >> r; return r; }
ULL rull() { ULL r; cin >> r; return r; }

std::string rstr() {std::string s; cin >> s; return s; }
