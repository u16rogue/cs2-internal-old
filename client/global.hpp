#pragma once

namespace global {

inline bool    obs_presence    = false;
inline void ** obs_present_ptr = nullptr;

namespace test {

inline bool  glow = false;
inline float rgb[4] = { 1.f, 1.f, 1.f };

inline float force_fov = -1.f;

inline bool force_sv_cheats = false;
inline bool force_sv_cheats_state = true;

inline bool no_smoke = false;

} // global::test

} // global
