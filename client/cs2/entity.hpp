#pragma once

#include <client/cs2/vecang.hpp>

namespace cs2 {

struct entity {
  void * vtable;

  inline auto get_eyepos(vec3 * out) -> vec3 * { // [06/04/2023] found at imgui debug trace, setup trace info
    return reinterpret_cast<vec3*(***)(void *, vec3 *)>(this)[0][154](this, out);
  }

  inline auto get_viewangles(ang3 * out) -> ang3 * { // [06/04/2023] found at imgui debug trace, setup trace info
    return reinterpret_cast<ang3*(***)(void *, ang3 *)>(this)[0][155](this, out);
  }
};

} // cs2
