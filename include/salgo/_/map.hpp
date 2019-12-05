#pragma once

#include "const-flag.hpp"

namespace salgo::_::map {

template<class Container, class Fun>
struct Params;


template<class P, Const_Flag C>
class Accessor;

template<class P>
class End_Iterator;

template<class P, Const_Flag C>
class Iterator;


template<class P>
struct Context;





template<class P>
class Map;

} // namespace salgo::_::map






namespace salgo {

template<class Container, class Fun>
class Map;

} // namespace salgo
