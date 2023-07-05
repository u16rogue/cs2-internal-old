#include "hooks.hpp"
#include "client/utils.hpp"
#include <common/mem.hpp>

#include <Windows.h>
#include <MinHook.h>
#include <Psapi.h>

#include <common/logging.hpp>
#include <common/types.hpp>
#include <common/utils.hpp>

#include <dxgi.h>
#include <d3d11.h>
#include <iterator>
#include <libloaderapi.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <winerror.h>
#include <winnt.h>
#include <winuser.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <client/cs2/convar.hpp>

#include <client/global.hpp>
#include <client/game.hpp>
#include <client/menu/menu.hpp>

#define def_hk(rt, nm, ...)                \
  static rt(*nm)(__VA_ARGS__) = nullptr;   \
  static auto __hk_##nm(__VA_ARGS__) -> rt

#define create_hk(nm, target) \
  (MH_CreateHook(target, reinterpret_cast<void *>(&__hk_##nm), reinterpret_cast<void **>(&nm)) == MH_OK)

// ---------------------------------------------------------------------------------------------------- 

def_hk(bool, cs2_can_send_packet, void * self) {
  if (GetAsyncKeyState(VK_DELETE)) {
    return false;
  }
  return cs2_can_send_packet(self);
}

def_hk(bool, cs2_spec_glow, void * unk1, void * unk2, i64 unk3, float * unk4, float * unk5, float * unk6, float * unk7, float * unk8, bool * unk9) {
  bool r = cs2_spec_glow(unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8, unk9);
  if (global::test::glow) {
    for (int i = 0; i < 3; ++i) {
      unk4[i] = global::test::rgb[i];
    }
    *unk6 = global::test::rgb[3];
    *unk9 = true;
    return true;
  }

  return r;
}

def_hk(float, cs2_get_player_fov, void * unk1) {
  if (global::test::force_fov != -1.f)
    return global::test::force_fov;
  return cs2_get_player_fov(unk1);
}

def_hk(bool, cs2_engine_get_sv_cheats_flag) {
  if (global::test::force_sv_cheats)
    return global::test::force_sv_cheats_state;
  return cs2_engine_get_sv_cheats_flag();
}

def_hk(void *, cs2_client_get_cvar_pvalue, cs2::convar_proxy * cvar, int flag) {
  if (global::test::force_sv_cheats && cvar->data == game::convar::sv_cheats) {
    return &global::test::force_sv_cheats_state;
  }

  if (menu::is_open() && (cvar->data == game::convar::imgui_enable_input || cvar->data == game::convar::imgui_enable)) {
    static bool xd = true;
    return &xd;
  }

  return cs2_client_get_cvar_pvalue(cvar, flag);
}

def_hk(void, cs2_client_set_cvar_value, cs2::convar_proxy * cvar, u64 flag, u64 value) {
  if (menu::is_open() && cvar->data == game::convar::stats_display && value == 5) {
    value = 0;
  }
  return cs2_client_set_cvar_value(cvar, flag, value);
}

def_hk(void, cs2_client_setup_draw_smoke, u8 * self) {
  if (global::test::no_smoke) {
    // *reinterpret_cast<u64 *>(self + 0x14) = 0;
    // *reinterpret_cast<u64 *>(self + 0x18) = 0;
    return;
  }
  return cs2_client_setup_draw_smoke(self);
}

// ---------------------------------------------------------------------------------------------------- 

static ID3D11RenderTargetView * dx_render_target_view = nullptr;

static auto cs2int_Present() -> void {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  menu::imgui_render(); 

  ImGui::Render();
  game::d3d_instance->device_context->OMSetRenderTargets(1, &dx_render_target_view, nullptr);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

def_hk(HRESULT, dxgi_Present, IDXGISwapChain * self, UINT SyncInterval, UINT Flags) {
  cs2int_Present();  
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

#if 0
def_hk(BOOL, _ClipCursor, const RECT *lpRect) {
  if (menu::is_open()) {
    if (!lpRect)
      return _ClipCursor(NULL); 
    return TRUE;
  }

  return _ClipCursor(lpRect);
}
#endif

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

  cs2log("Renderer ready!");
  return true;
}

static decltype(dxgi_Present) obs_Present = nullptr;
static auto obsproxy_dxgi_Present(IDXGISwapChain * self, UINT SyncInterval, UINT Flags) -> HRESULT {
  if (global::obs_presence) {
    cs2int_Present();
  }
  return obs_Present(self, SyncInterval, Flags);
}

def_hk(HRESULT, obs2gameoverlay_dxgi_Present, IDXGISwapChain * self, UINT SyncInterval, UINT Flags) {
  if (!global::obs_presence) {
    cs2int_Present();
  }
  return obs2gameoverlay_dxgi_Present(self, SyncInterval, Flags);
}

static auto obs_present_fix(void *& target) -> bool {
  HMODULE graphicshook = GetModuleHandleA("graphics-hook64.dll");
  if (!graphicshook)
    return false;

  bool failed = true;
  mpp_defer {
    if (failed) {
      cs2log("OBS fix failed! Falling back...");
    }
  };

  cs2log("OBS detected! Applying OBS fix...");

  MODULEINFO mi {};
  if (!GetModuleInformation(GetCurrentProcess(), graphicshook, &mi, sizeof(mi))) {
    cs2log("Cant gather module info.");
    return false;
  }

  u8 * fnpt = reinterpret_cast<u8 *>(target);
  if (fnpt[0] != 0xE9) {
    cs2log("Expected x86 relative jump on function, non found. (E9 00000000)");
    return false;
  }

  u8 * tramp_proxy = reinterpret_cast<u8 *>(utils::rel2abs(fnpt, 1));
  if (*reinterpret_cast<u16 *>(tramp_proxy) != *reinterpret_cast<const u16 *>("\xFF\x25")) {
    cs2log("Expected x64 ptr jump on trampoline proxy, non found. (FF25 00000000)");
    return false;
  }

  if ((tramp_proxy - 0x5)[0] != 0xE9) {
    cs2log("Expected x86 relative jump on trampoline original proxy to gameoverlayrenderer, non found. (E9 00000000)");
    return false;
  }

  global::obs_present_ptr = reinterpret_cast<void **>(utils::rel2abs(tramp_proxy, 2));
  cs2log("OBS Present pp: {}", (void *)global::obs_present_ptr);

  DWORD oprot = 0;
  if (!VirtualProtect(global::obs_present_ptr, sizeof(uiptr), PAGE_EXECUTE_READWRITE, &oprot)) {
    cs2log("Failed to change R* to RWX...");
    global::obs_present_ptr = nullptr;
    return false;
  }

  mpp_defer {
    if (!VirtualProtect(global::obs_present_ptr, sizeof(uiptr), oprot, &oprot))
      cs2log("Failed to reapply original protection. Ignoring but this can become an issue.");
  };

  if (!create_hk(obs2gameoverlay_dxgi_Present, tramp_proxy - 5)) {
    cs2log("Failed to hook proxy from obs to gameoverlay");
    return false;
  }

  obs_Present = reinterpret_cast<decltype(dxgi_Present)>(*global::obs_present_ptr);
  *global::obs_present_ptr = reinterpret_cast<void *>(&obsproxy_dxgi_Present);

  failed = false;
  cs2log("OBS fix applied!");
  return true;
}

static auto post_entry_fix_present_hk(void *& target) -> bool {
  return false;

#if 0 // [01/04/2023] stack corruption, maybe in the future ill fix this
  constexpr int jump_target_offset = 10;

  static u8 sh_Present_original[] = {
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov     rax, <cs2int_Present>
    0xFF, 0xD0,                                                 // call    rax
    0x55,                                                       // push    rbp
    0x57,                                                       // push    rdi
    0x41, 0x56,                                                 // push    r14
    0x48, 0x8D, 0x6C, 0x24, 0x90,                               // lea     rbp, [rsp-70h]  
    0x48, 0x81, 0xEC, 0x70, 0x01, 0x00, 0x00,                   // sub     rsp, 170h 
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov     rax, <dxgi.Present + sizeof(sh_Present_original)>
    0xFF, 0xE0,                                                 // jmp     rax
  };

  u8 * p = reinterpret_cast<u8 *>(target);
  for (int i = 0; i < 16; ++i) {
    if (p[jump_target_offset + i] != sh_Present_original[i + 12]) {
      cs2log("Mismatch of expected bytes for DXGI.Present.");
      return false;
    }
  }

  if (DWORD oprot = 0; !VirtualProtect(reinterpret_cast<void *>(&sh_Present_original), sizeof(sh_Present_original), PAGE_EXECUTE_READWRITE, &oprot)) {
    cs2log("Failed to change shell protection.");
    return false;
  }

  *reinterpret_cast<void **>(sh_Present_original + 2)  = reinterpret_cast<void *>(&cs2int_Present);
  *reinterpret_cast<void **>(sh_Present_original + 30) = p + 0x1A;

  u8 * jmp_target = p + jump_target_offset;
  u8 jmp_shell[] = {
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov     rax, <sh_Present_original>
    0xFF, 0xE0,                                                 // jmp     rax
 };

  *reinterpret_cast<void **>(jmp_shell + 2) = &sh_Present_original;

  DWORD oprot = 0;
  if (!VirtualProtect(jmp_target, sizeof(jmp_shell), PAGE_EXECUTE_READWRITE, &oprot)) {
    cs2log("Failed to change DXGI.Present protection for patching.");
    return false;
  }

  mpp_defer {
    if (!VirtualProtect(jmp_target, sizeof(jmp_shell), oprot, &oprot))
      cs2log("Failed to reapply original protection. Ignoring but this can become an issue.");
  };

  for (int i = 0; i < sizeof(jmp_shell); ++i) {
    jmp_target[i] = jmp_shell[i];
  }

  return true;
#endif
}

auto hooks::install() -> bool {
  cs2log("Installing hooks...");

  if (MH_Initialize() != MH_OK) {
    cs2log("Failed to initialize MinHook!");
    return false;
  }

  u8 * client = game::so::client.get_base<u8 *>();
  u8 * engine = game::so::engine.get_base<u8 *>();
  u8 * netsys = game::so::netsys.get_base<u8 *>();

  cs2log("Hooking cs2_spec_glow...");
  if (!create_hk(cs2_spec_glow, client + 0x77A470)) {
    cs2log("Failed to hook cs2_spec_glow");
    return false;
  }

  cs2log("Hooking cs2_get_player_fov...");
  if (!create_hk(cs2_get_player_fov, client + 0x4A7430)) {
    cs2log("Failed to hook cs2_get_player_fov");
    return false;
  }

  cs2log("Hooking cs2_client_get_cvar_value...");
  if (!create_hk(cs2_client_get_cvar_pvalue, client + 0xED3B70)) {
    cs2log("Failed to hook cs2_client_get_cvar_value");
    return false;
  }

  cs2log("Hooking cs2_client_set_cvar_value...");
  if (!create_hk(cs2_client_set_cvar_value, client + 0xFBE00)) {
    cs2log("Failed to hook cs2_client_set_cvar_value");
    return false;
  }

  cs2log("Hooking cs2_engine_get_sv_cheats_flag...");
  if (!create_hk(cs2_engine_get_sv_cheats_flag, engine + 0xD8EC0)) {
    cs2log("Failed to hook cs2_engine_get_sv_cheats_flag");
    return false;
  } 

  cs2log("Hooking cs2_client_setup_draw_smoke...");
  if (!create_hk(cs2_client_setup_draw_smoke, client + 0x41BC00)) {
    cs2log("Failed to hook cs2_client_setup_draw_smoke");
    return false;
  }

  cs2log("Hooking cs2_can_send_packet...");
  if (!create_hk(cs2_can_send_packet, netsys + 0x7F660)) {
    cs2log("Failed to hook cs2_can_send_packet");
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

#if 0
  cs2log("Hooking ClipCursor...");
  if (!create_hk(_ClipCursor, reinterpret_cast<void **>(&ClipCursor))) {
    cs2log("Failed to hook ClipCursor");
    return false;
  }
#endif

  cs2log("Hooking dxgi.Present...");
  if (void * dxgi_Present_target = reinterpret_cast<void ***>(game::d3d_instance->info->swapchain)[0][8]; dxgi_Present_target) {
    cs2log("dxgi.Present @ {}", dxgi_Present_target);
    if (!post_entry_fix_present_hk(dxgi_Present_target) && !obs_present_fix(dxgi_Present_target) && !create_hk(dxgi_Present, dxgi_Present_target)) {
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
  if (global::obs_present_ptr) {
    DWORD oprot = 0;
    VirtualProtect(global::obs_present_ptr, sizeof(uiptr), PAGE_EXECUTE_READWRITE, &oprot);
    *global::obs_present_ptr = reinterpret_cast<void **>(obs_Present);
    VirtualProtect(global::obs_present_ptr, sizeof(uiptr), oprot, &oprot);
  }
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
