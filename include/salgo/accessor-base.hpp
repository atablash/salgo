#pragma once

#include "common.hpp"
#include "const-flag.hpp"

#include <glog/logging.h>

namespace salgo {







template< Const_Flag C, template<Const_Flag> class CRTP >
struct Accessor_Base {
	
	// conversion operator to HANDLE
	operator auto() const { return _self().handle(); }

	// conversion operator to VAL
	operator auto&()       { return _self()(); }
	operator auto&() const { return _self()(); }

	CRTP_COMMON( Accessor_Base, CRTP<C> )
};




} // namespace salgo


