#include <common/mem.hpp>

auto common::mem::details::pattern_scan(void * start, const usize size, const char * pattern, const usize pattern_length, const u8 mask) -> void * {
  u8 *       base = reinterpret_cast<u8 *>(start);
  const u8 * patt = reinterpret_cast<const u8 *>(pattern);

  for (int offset = 0; offset < size - pattern_length; ++offset) {
    for (int poff = 0; poff < pattern_length; ++poff) {

      u8 cbyte = base[offset + poff];
      u8 cpatt = patt[poff];

      if (cpatt == mask)
        continue;
      else if (cbyte != cpatt)
        break;
      else if (poff == pattern_length - 1)
        return base + offset;
    }
  }
  return nullptr;
}
