#include "hooks.hpp"
#include "common/pattern_scanner.hpp"

#include <Windows.h>
#include <MinHook.h>

#include <common/logging.hpp>
#include <common/types.hpp>
#include <common/utils.hpp>

#include <dxgi.h>
#include <d3d11.h>
#include <winerror.h>
#include <winuser.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "global.hpp"
#include "game.hpp"
#include "menu/menu.hpp"

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
#if 0
  bool r = cs2_spec_glow(unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8, unk9);
  for (int i = 0; i < 3; ++i) {
    unk4[i] = global::test::rgb[i];
  }
  *unk5 = global::test::unk0;
  *unk6 = global::test::unk1;
  *unk9 = true;
  return true;
#endif

  return cs2_spec_glow(unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8, unk9);
}

// ---------------------------------------------------------------------------------------------------- 

static ID3D11RenderTargetView * dx_render_target_view = nullptr;

def_hk(HRESULT, dxgi_Present, IDXGISwapChain * self, UINT SyncInterval, UINT Flags) {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  menu::imgui_render(); 

  ImGui::Render();
  game::d3d_instance->device_context->OMSetRenderTargets(1, &dx_render_target_view, nullptr);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  return dxgi_Present(self, SyncInterval, Flags);
}

def_hk(HRESULT, dxgi_ResizeBuffers, IDXGISwapChain * self, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
  if (dx_render_target_view) {
    dx_render_target_view->Release();
    dx_render_target_view = nullptr;
  }

  auto result = dxgi_ResizeBuffers(self, BufferCount, Width, Height, NewFormat, SwapChainFlags);

  if (ID3D11Texture2D * dxbb = nullptr; SUCCEEDED(game::d3d_instance->info->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&dxbb)))) {
    if (FAILED(game::d3d_instance->device->CreateRenderTargetView(dxbb, nullptr, &dx_render_target_view))) {
      cs2log("Failed to recreate RenderTarget");
    }
    dxbb->Release();
  } else {
    cs2log("Failed to recreate BackBuffer");
  }

  return result;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
def_hk(LRESULT, wndproc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

  if (msg == WM_KEYDOWN && wparam == VK_INSERT) {
    menu::toggle();
  }

  if (menu::is_open() && (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam) || menu::wndproc(hwnd, msg, wparam, lparam))) {
    return TRUE;
  }

  return wndproc(hwnd, msg, wparam, lparam);
}

def_hk(BOOL, _ClipCursor, const RECT *lpRect) {
  if (menu::is_open()) {
    if (!lpRect)
      return _ClipCursor(NULL); 
    return TRUE;
  }

  return _ClipCursor(lpRect);
}

// ---------------------------------------------------------------------------------------------------- 

static auto prep_render() -> bool {
  cs2log("Preparing renderer...");

  ImGui::CreateContext();
  ImGui::GetIO().IniFilename = nullptr;

  cs2log("Acquiring BackBuffer...");
  ID3D11Texture2D * dx_backbuffer = nullptr;
  if (FAILED(game::d3d_instance->info->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&dx_backbuffer)))) {
    cs2log("Failed to create BackBuffer");
    return false;
  }

  mpp_defer {
    if (dx_backbuffer) {
      dx_backbuffer->Release();
      dx_backbuffer = nullptr;
    }
  };

  cs2log("Acquiring RenderTarget");
  if (FAILED(game::d3d_instance->device->CreateRenderTargetView(dx_backbuffer, nullptr, &dx_render_target_view))) {
    cs2log("Failed to create RenderTarget");
    return false;
  }

  cs2log("Initializing ImGui...");
  if (!ImGui_ImplWin32_Init(game::d3d_instance->info->window) || !ImGui_ImplDX11_Init(game::d3d_instance->device, game::d3d_instance->device_context)) {
    cs2log("Failed to initialize ImGui.");
  }

  cs2log("Renderer ready!1!! :3");
  return true;
}

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
    return false;
  }
 
  if (!prep_render()) {
    return false;
  }

  cs2log("Hooking WndProc...");
  if (void * wndproc_target = reinterpret_cast<void *>(GetWindowLongPtrW(game::d3d_instance->info->window, GWLP_WNDPROC)); wndproc_target) {
    cs2log("WndProc @ {}", wndproc_target);
    if (!create_hk(wndproc, wndproc_target)) {
      cs2log("Failed to hook WndProc");
      return false;
    }
  } else {
    cs2log("WndProc not found.");
    return false;
  }

  cs2log("Hooking ClipCursor...");
  if (!create_hk(_ClipCursor, reinterpret_cast<void **>(&ClipCursor))) {
    cs2log("Failed to hook ClipCursor");
    return false;
  }


  cs2log("Hooking dxgi.Present...");
  if (void * dxgi_Present_target = reinterpret_cast<void ***>(game::d3d_instance->info->swapchain)[0][8]; dxgi_Present_target) {
    cs2log("dxgi.Present @ {}", dxgi_Present_target);
    if (!create_hk(dxgi_Present, dxgi_Present_target)) {
      cs2log("Failed to hook dxgi.Present");
      return false;
    }
  } else {
    cs2log("dxgi.Present not found.");
    return false;
  }

  cs2log("Hooking dxgi.ResizeBuffers...");
  if (void * dxgi_ResizeBuffers_target = reinterpret_cast<void ***>(game::d3d_instance->info->swapchain)[0][13]; dxgi_ResizeBuffers_target) {
    cs2log("dxgi.ResizeBuffers @ {}", dxgi_ResizeBuffers_target);
    if (!create_hk(dxgi_ResizeBuffers, dxgi_ResizeBuffers_target)) {
      cs2log("Failed to hook dxgi.ResizeBuffers");
      return false;
    }
  } else {
    cs2log("dxgi.ResizeBuffers not found.");
    return false;
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

  // [27-03-2023] We intentionally unhook first before uninit so we dont have hooks trying to use imgui
  cs2log("Shutting down ImGui...");
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
  cs2log("ImGui shutted down.");

  if (dx_render_target_view)
    dx_render_target_view->Release();

  return true;
}

#undef make_module_info
#undef create_hk
#undef def_hk
