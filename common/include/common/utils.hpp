#pragma once

#include <tuple>
#include <common/types.hpp>

namespace common::utils {

auto module_info(const char * nm) -> std::pair<u8 *, usize>;

} // common::utils
