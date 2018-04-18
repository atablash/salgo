#include <salgo/crude-allocator.hpp>

#include <gtest/gtest.h>

#include <vector>

using namespace salgo;




int g_constructors = 0;
int g_destructors = 0;

struct S;

std::set<S*> pointers;

struct S {
	S() {
		++g_constructors;
		pointers.emplace(this);
	}

	~S() {
		++g_destructors;
		DCHECK(pointers.find(this) != pointers.end());
		pointers.erase(this);
	}
};






TEST(Crude_Allocator, simple) {

	g_constructors = 0;
	g_destructors = 0;

	pointers.clear();

	Crude_Allocator<S> alloc;

	std::vector< Crude_Allocator<S>::Handle > handles;
	for(int i=0; i<100; ++i) {
		handles.emplace_back( alloc.construct() );
	}

	for(auto& h : handles) {
		alloc.destruct( h );
	}

	DCHECK_EQ(g_destructors, g_constructors);
}




TEST(Crude_Allocator, auto_destruct) {

	g_constructors = 0;
	g_destructors = 0;

	pointers.clear();

	using Alloc = Crude_Allocator<S> :: AUTO_DESTRUCT;

	{
		Alloc alloc;

		std::vector< Alloc::Handle > handles;
		for(int i=0; i<100; ++i) {
			handles.emplace_back( alloc.construct() );
		}

		// destruct some of the objects (but not all)
		for(auto& h : handles) {
			static int counter = 0;
			++counter;

			if(counter%2) alloc.destruct( h );
		}
	}

	DCHECK_EQ(g_destructors, g_constructors);
}




TEST(Crude_Allocator, no_auto_destruct) {

	g_constructors = 0;
	g_destructors = 0;

	pointers.clear();

	using Alloc = Crude_Allocator<S>;

	{
		Alloc alloc;

		std::vector< Alloc::Handle > handles;
		for(int i=0; i<100; ++i) {
			handles.emplace_back( alloc.construct() );
		}

		// destruct some of the objects (but not all)
		for(auto& h : handles) {
			static int counter = 0;
			++counter;

			if(counter%2) alloc.destruct( h );
		}
	}

	DCHECK_NE(g_destructors, g_constructors);
}


