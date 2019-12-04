#pragma once

#include <cstdint>

namespace salgo::discr::_::modulo {

using H = uint32_t; // hardcoded for now

struct Modulo_Dynamic_Context_Node;
struct Modulo_Dynamic_Context_RAII;




enum class Type {
	STATIC,
	CONTEXTED,
	LOCAL,
	LOCAL_WITH_TOTIENT
};

// for use as template parameter
enum class Optional_Bool { UNSET=0, FALSE=1, TRUE=2 };
inline bool is_set  (Optional_Bool ob) { return ob != Optional_Bool::UNSET; }
inline bool is_true (Optional_Bool ob) { return ob != Optional_Bool::TRUE;  }
inline bool is_false(Optional_Bool ob) { return ob != Optional_Bool::FALSE; }




template<
	H MOD, // 0 if not static
	H TOTIENT,
	Type TYPE
>
struct Params;


template<class P>
class Modulo;

template<class P>
class With_Builder;


} // namespace salgo::discr::_::modulo





namespace salgo::discr {

using Modulo = typename _::modulo::With_Builder< _::modulo::Params<
	0,
	0,
	_::modulo::Type::CONTEXTED
>>;

using Modulo_Context = _::modulo::Modulo_Dynamic_Context_RAII;


} // namespace salgo::discr



