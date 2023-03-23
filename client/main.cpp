#include <Windows.h>
#include <common/logging.hpp>
#include <common/types.hpp>
#include <cstdio>
#include <metapp/metapp.hpp>

struct intfreg {
  void * (*create)(void);
  const char * name;
  intfreg *    next;
};

auto WINAPI init_thread(LPVOID arg) -> DWORD {
#if defined(CS2INT_COMMON_LOGGING) && CS2INT_COMMON_LOGGING == 1
  {
    AllocConsole();
    FILE * f;
    freopen_s(&f, "CONOUT$", "w", stdout);
  }
#endif

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
      cs2log("\nInteface: {}", intf->name);
    }

    return;
  };

  cs2log("Start dump..."); 
  dump_module_interface("client.dll");
  dump_module_interface("engine2.dll");

  return 0;
}

auto WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) -> BOOL {
  if (reason == DLL_PROCESS_ATTACH) {
    HANDLE h = CreateThread(NULL, NULL, init_thread, instance, NULL, NULL);
    mpp_defer { CloseHandle(h); };
  } else if (reason == DLL_PROCESS_DETACH) {
#if defined(CS2INT_COMMON_LOGGING) && CS2INT_COMMON_LOGGING == 1
    FreeConsole();
#endif
  }
  return TRUE;
}
