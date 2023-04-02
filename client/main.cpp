#include <Windows.h>
#include <common/logging.hpp>
#include <common/types.hpp>
#include <cstdio>

#include <metapp/metapp.hpp>
// #include <kahel-winpe/kahel-winpe.hpp>
#include <client/utils.hpp>

#include "client/utils.hpp"
#include "game.hpp"
#include "hooks.hpp"

#if defined(CS2INT_COMMON_LOGGING) && CS2INT_COMMON_LOGGING == 1
#define IS_LOGGING(...)
#else
#define IS_LOGGING(...) (void)[]()
#endif

static auto WINAPI init_thread(LPVOID arg) -> DWORD {
  IS_LOGGING() {
    AllocConsole();
    FILE * f;
    freopen_s(&f, "CONOUT$", "w", stdout);
  };

  game::init() && hooks::install();

  return 0;
}

auto WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) -> BOOL {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(instance);
    HANDLE h = CreateThread(NULL, NULL, init_thread, instance, NULL, NULL);
    mpp_defer { CloseHandle(h); };
  } else if (reason == DLL_PROCESS_DETACH) {
    hooks::uninstall();
    game::uninit();
    IS_LOGGING() { FreeConsole(); };
  }
  return TRUE;
}

#undef IS_LOGGING
