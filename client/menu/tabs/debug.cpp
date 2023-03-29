#include "debug.hpp"

#include <imgui.h>
#include <vector>
#include <tuple>
#include <Windows.h>
#include <common/logging.hpp>
#include "../../utils.hpp"

static auto interface_dump() -> void {
  if (!ImGui::BeginTabItem("Interface"))
    return;

  static char modname[256] = { "client.dll" };
  static std::vector<std::tuple<const char *, void *>> dump;

  ImGui::InputText("Module Target", modname, sizeof(modname));
  ImGui::SameLine();
  if (ImGui::Button("Dump")) {
    HMODULE target = GetModuleHandleA(modname);
    if (target) {
      for (auto intf : utils::get_module_interfaces(target)) {
        dump.emplace_back(std::make_tuple(intf->name, intf->create()));
      }
    } else {
      cs2log("Invalid module name.");
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    dump.clear();
  }

  if (ImGui::BeginTable("##cs2interfacedumptable", 2, ImGuiTableFlags_ScrollY, { 0.f, ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - 20.f })) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Interface");
    ImGui::TableHeadersRow();

    for (auto [name, ptr] : dump) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", name);
      ImGui::TableNextColumn();
      ImGui::Text("0x%p", ptr);
    }

    ImGui::EndTable();
  }

  ImGui::EndTabItem();
}

auto menu::tab::debug_tab::on_menu_tab() -> void {
  if (!ImGui::BeginTabBar("##cs2tabdebug"))
    return;

  interface_dump();

  ImGui::EndTabBar();
}
