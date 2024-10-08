#pragma once

#if defined(CS2INT_COMMON_LOGGING) && CS2INT_COMMON_LOGGING == 1

  #include <cstdio>
  #include <format>
  #include <string_view>

namespace common {

namespace details { constexpr int cs2log_offset = sizeof(CS2INT_COMMON_SOURCE_DIR); } // common::details

template <typename... vargs_t>
auto cs2log(const char * loginfo, std::string_view fmt, vargs_t... vargs) -> void {
  printf("\n[cs2int|%s] %s", loginfo, std::vformat(fmt, std::make_format_args(vargs...)).c_str());
}

} // namespace cs2::common

  #define _cs2stringify(x) #x
  #define cs2stringify(x) _cs2stringify(x)

  #define cs2log(...) \
    common::cs2log((__FILE__ ":" cs2stringify(__LINE__)) + common::details::cs2log_offset, __VA_ARGS__)

#else // defined(CS2INT_COMMON_LOGGING) && CS2INT_COMMON_LOGGING == 1

  #define cs2log(...)

#endif // defined(CS2INT_COMMON_LOGGING) && CS2INT_COMMON_LOGGING == 1
