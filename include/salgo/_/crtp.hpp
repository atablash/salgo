#pragma once


#define SALGO_CRTP_COMMON(DERIVED)\
  private:\
	auto& _self()	    { return *static_cast<	     DERIVED* >(this); }\
	auto& _self() const { return *static_cast< const DERIVED* >(this); }

