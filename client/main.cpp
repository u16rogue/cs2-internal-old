#include <Windows.h>
#include <common/logging.hpp>
#include <common/types.hpp>
#include <cstdio>

#include <metapp/metapp.hpp>
// #include <kahel-winpe/kahel-winpe.hpp>

#include "game.hpp"
#include "hooks.hpp"

#if defined(CS2INT_COMMON_LOGGING) && CS2INT_COMMON_LOGGING == 1
#define IS_LOGGING(...)
#else
#define IS_LOGGING(...) (void)[]()
#endif

struct intfreg {
  void * (*create)(void);
  const char * name;
  intfreg *    next;
};

static auto dump_interface() -> void {
const auto dump_module_interface = [](const char * mname) {
    HMODULE hm = GetModuleHandleA(mname);
    if (!hm) {
      cs2log("{} not found.", mname);
      return;
    }
    cs2log("{} @ {}", mname, (void *)hm);

    u8 * createintf_export = reinterpret_cast<decltype(createintf_export)>(GetProcAddress(hm, "CreateInterface"));
    if (!createintf_export) {
      cs2log("CreateInterface export not found.");
      return;
    }
    cs2log("{}.CreateInterface @ {}", mname, (void *)createintf_export);

    idiff displacement = *reinterpret_cast<idiff *>(createintf_export + 0x3);
    cs2log("Displacement: {}", displacement);

    intfreg * intf = *reinterpret_cast<intfreg **>((createintf_export + 0x7) + displacement);
    for (; intf; intf = intf->next) {
      cs2log("\tInteface: {} @ {}", intf->name, intf->create());
    }

    return;
  };

  cs2log("Start dump..."); 
  for (auto m : {
      "client.dll", "engine2.dll", "panorama.dll", "panoramauiclient.dll"
    }) {
    dump_module_interface(m);
  }
}


static auto WINAPI init_thread(LPVOID arg) -> DWORD {
  IS_LOGGING() {
    AllocConsole();
    FILE * f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    dump_interface();
  };

  game::init();
  hooks::install();

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
