#include "hooks.hpp"

#include <Windows.h>
#include <MinHook.h>

#include <common/logging.hpp>
#include <common/types.hpp>

#include <dxgi.h>
#include <d3d11.h>

#define def_hk(rt, nm, ...)                 \
  static rt(*nm)(__VA_ARGS__) = nullptr;   \
  static auto __hk_##nm(__VA_ARGS__) -> rt

#define create_hk(nm, target) \
  (MH_CreateHook(target, reinterpret_cast<void *>(&__hk_##nm), reinterpret_cast<void **>(&nm)) == MH_OK)

// ---------------------------------------------------------------------------------------------------- 

def_hk(bool, cs2_spec_glow, void * unk1, void * unk2, i64 unk3, float * unk4, float * unk5, float * unk6, float * unk7, float * unk8, bool * unk9) {
  bool r = cs2_spec_glow(unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8, unk9);
  unk4[0] = 1.f;
  unk4[1] = 1.f;
  unk4[2] = 1.f;
  *unk5 = 1.f;
  *unk6 = 1.f;
  *unk9 = true;
  return true;
}

def_hk(HRESULT, dx_Present, IDXGISwapChain * self, UINT SyncInterval, UINT Flags) {
  return dx_Present(self, SyncInterval, Flags);
}

// ---------------------------------------------------------------------------------------------------- 

auto hooks::install() -> bool {
  cs2log("Installing hooks...");

  if (MH_Initialize() != MH_OK) {
    cs2log("Failed to initialize MinHook!");
    return false;
  }

  u8 * client_base = reinterpret_cast<decltype(client_base)>(GetModuleHandleA("client.dll"));

  cs2log("Hooking cs2_spec_glow...");
  if (!create_hk(cs2_spec_glow, client_base + 0x77A470)) {
    cs2log("Failed to hook cs2_spec_glow");
  }

  cs2log("Enabling all hooks...");
  if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
    cs2log("Failed to enable all hooks!");
    return false;
  }

  return true;
}

auto hooks::uninstall() -> bool {
  cs2log("Uninstalling hooks...");
  MH_DisableHook(MH_ALL_HOOKS);
  MH_Uninitialize();
  return true;
}

#undef create_hk
#undef def_hk
