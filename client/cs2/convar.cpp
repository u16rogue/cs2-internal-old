#include "convar.hpp"

auto cs2::FCVAR::tocstr(type id) -> const char * {
  #define _CS2_FCVAR_ENTRY(cvar, nm) case nm: return "FCVAR_" #cvar;
  switch (id) {
    #include "fcvar_defs.lst"
  }
  #undef _CS2_FCVAR_ENTRY
  return "<unknown FCVAR>";
}
