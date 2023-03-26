#pragma once

#include <tuple>
#include <common/types.hpp>

namespace common::utils {

auto module_info(const char * nm) -> std::pair<u8 *, usize>;

template <typename T, int lvl> requires (lvl > 1)
struct lazy_arrow {
  lazy_arrow(void * ptr_) : ptr(ptr_) {}

  auto operator->() -> T * {
    void * result = ptr;
    for (int i = 0; i < lvl - 1; ++i) {
      result = *reinterpret_cast<void **>(result);
    }
    return reinterpret_cast<T *>(result);
  }

  void * ptr;

};
static_assert(sizeof(lazy_arrow<void, 2>) == sizeof(void *), "Invalid size of lazy_arrow instance expecting it to be the same as SIZE OF P");

} // common::utils
