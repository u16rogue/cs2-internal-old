#pragma once

#include "imgui.h"
#include "tabs/debug.hpp"
#include <imgui.h>

namespace menu {
namespace details {
template <typename... tabs>
struct _tabs_dispatcher {

  static auto on_menu_tab() -> void {
    ([](){
      if constexpr (requires { tabs::on_menu_tab(); }) {
        ImGui::BeginTabItem(tabs::title);
        tabs::on_menu_tab();
        ImGui::EndTabItem();
      }
    }(), ...);
  }

};
} // details

using tabs = details::_tabs_dispatcher<
  tab::debug_tab
>;

}
