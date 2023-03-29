#pragma once

#include <Windows.h>

namespace menu {

auto is_open() -> bool;
auto toggle(int v = -1) -> void;

auto imgui_render() -> void;
auto wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> bool;

} // menu
