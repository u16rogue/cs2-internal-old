#include "menu.hpp"
#include <imgui.h>
#include <winuser.h>
#include <Windows.h>

#include <client/game.hpp>

#include <client/menu/tabs.hpp>

static bool menu_open = false;
static bool last_cflag = false;

auto menu::is_open() -> bool {
  return menu_open;
}

auto menu::toggle(int v) -> void {
  bool prev_menu_open = menu_open;
  if (v == -1) {
    menu_open = !menu_open;
  } else {
    menu_open = v;
  }

#if 0
  // TODO: lmao, implement this better
  if (menu_open) {
    ClipCursor(nullptr);
    CURSORINFO ci = { .cbSize = sizeof(ci) };
    GetCursorInfo(&ci);
    last_cflag = ci.flags == 1;
  } else {
    if (last_cflag) {
      POINT tl    = { 0, 0 };
      ClientToScreen(game::d3d_instance->info->window, &tl); 
      RECT clsz = {};
      GetClientRect(game::d3d_instance->info->window, &clsz);
      POINT br = { .x = clsz.right, .y = clsz.bottom };
      ClientToScreen(game::d3d_instance->info->window, &br);
      RECT clip = { .left = tl.x, .top = tl.y, .right = br.x, .bottom = br.y };
      ClipCursor(&clip);
    } 
  }

  ImGui::GetIO().MouseDrawCursor = menu_open || !last_cflag;
#endif
}

auto menu::imgui_render() -> void {
  auto * fg = ImGui::GetBackgroundDrawList();
  fg->AddText(ImVec2(3.f, 3.f), 0xFF000000, CS2_EXTERNAL_NAME);
  fg->AddText(ImVec2(2.f, 2.f), 0xFFFFFFFF, CS2_EXTERNAL_NAME);

  if (!menu_open)
    return;

  static auto _ = [] {
    auto wh = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(ImVec2(wh.x * 0.8, wh.y * 0.6), ImGuiCond_::ImGuiCond_FirstUseEver);
    return 0;
  }();

  ImGui::Begin(CS2_EXTERNAL_NAME);
  ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

  if (ImGui::BeginTabBar("##cs2tabs")) {
    tabs::on_menu_tab();
    ImGui::EndTabBar();
  }

  ImGui::End();
}

auto menu::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> bool {
  if (menu_open) {
    switch (msg) {
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_MOUSEMOVE:
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_LBUTTONDBLCLK:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_RBUTTONDBLCLK:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_MBUTTONDBLCLK:
        return true;
    }
  }
  return false;
}
