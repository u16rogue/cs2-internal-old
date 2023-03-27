#include "hooks.hpp"
#include "common/pattern_scanner.hpp"

#include <Windows.h>
#include <MinHook.h>

#include <common/logging.hpp>
#include <common/types.hpp>
#include <common/utils.hpp>

#include <dxgi.h>
#include <d3d11.h>
#include <winuser.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "game.hpp"

#define make_module_info(id, nm)                                      \
  auto [id, id##_sz] = common::utils::module_info(nm);                \
  if (!id || !id##_sz) {                                              \
    cs2log(nm " not found.");                                         \
    return false;                                                     \
  }                                                                   \
  cs2log(nm " @ {} ({} bytes)", reinterpret_cast<void *>(id), id##_sz)

#define def_hk(rt, nm, ...)                \
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

def_hk(HRESULT, dxgi_Present, IDXGISwapChain * self, UINT SyncInterval, UINT Flags) {
  return dxgi_Present(self, SyncInterval, Flags);
}

def_hk(HRESULT, dxgi_ResizeBuffers, IDXGISwapChain * self, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
  return dxgi_ResizeBuffers(self, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

def_hk(LRESULT, wndproc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return wndproc(hwnd, msg, wparam, lparam);
}

// ---------------------------------------------------------------------------------------------------- 

auto hooks::install() -> bool {
  cs2log("Installing hooks...");

  if (MH_Initialize() != MH_OK) {
    cs2log("Failed to initialize MinHook!");
    return false;
  }

  make_module_info(client, "client.dll");

  cs2log("Hooking cs2_spec_glow...");
  if (!create_hk(cs2_spec_glow, client + 0x77A470)) {
    cs2log("Failed to hook cs2_spec_glow");
  }

  cs2log("Hooking WndProc...");
  if (void * wndproc_target = reinterpret_cast<void *>(GetWindowLongPtrW(game::d3d_instance->info->window, GWLP_WNDPROC)); wndproc_target) {
    cs2log("WndProc @ {}", wndproc_target);
    if (!create_hk(wndproc, wndproc_target)) {
      cs2log("Failed to hook WndProc");
    }
  } else {
    cs2log("WndProc not found.");
  }

  cs2log("Hooking dxgi.Present...");
  if (void * dxgi_Present_target = reinterpret_cast<void ***>(game::d3d_instance->info->swapchain)[0][8]; dxgi_Present_target) {
    cs2log("dxgi.Present @ {}", dxgi_Present_target);
    if (!create_hk(dxgi_Present, dxgi_Present_target)) {
      cs2log("Failed to hook dxgi.Present");
    }
  } else {
    cs2log("dxgi.Present not found.");
  }

  cs2log("Hooking dxgi.ResizeBuffers...");
  if (void * dxgi_ResizeBuffers_target = reinterpret_cast<void ***>(game::d3d_instance->info->swapchain)[0][13]; dxgi_ResizeBuffers_target) {
    cs2log("dxgi.ResizeBuffers @ {}", dxgi_ResizeBuffers_target);
    if (!create_hk(dxgi_ResizeBuffers, dxgi_ResizeBuffers_target)) {
      cs2log("Failed to hook dxgi.ResizeBuffers");
    }
  } else {
    cs2log("dxgi.ResizeBuffers not found.");
  }
  
  cs2log("Enabling all hooks...");
  if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
    cs2log("Failed to enable all hooks!");
    return false;
  }

  cs2log("Hook installation finished!");
  return true;
}

auto hooks::uninstall() -> bool {
  cs2log("Uninstalling hooks...");
  MH_DisableHook(MH_ALL_HOOKS);
  MH_Uninitialize();
  cs2log("Hooks uninstalled!");
  return true;
}

#undef make_module_info
#undef create_hk
#undef def_hk
