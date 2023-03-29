#pragma once

#include <common/types.hpp>

namespace cs2 {

// CUtlVector
template <typename T>
struct list { 
  usize length;
  T *   data;
};
static_assert(sizeof(list<void>) == 0x10, "Invalid cs2::list<T>");

}
