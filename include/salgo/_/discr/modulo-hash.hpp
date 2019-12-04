#pragma once

#include "modulo.hpp"



namespace salgo::discr::_::modulo_hash {


template<int NUM_HASHES>
class Modulo_Hash;



} // namespace salgo::discr::_::modulo_hash





namespace salgo::discr {

template<int NUM_HASHES>
using Modulo_Hash = _::modulo_hash::Modulo_Hash<NUM_HASHES>;


} // namespace salgo::discr
