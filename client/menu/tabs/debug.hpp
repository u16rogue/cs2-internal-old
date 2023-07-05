#pragma once

namespace menu::tab {

struct debug_tab {

static constexpr char title[] = "Debug";
static auto on_menu_tab() -> void;
static auto on_imgui() -> void;

};

} // menu::tab
