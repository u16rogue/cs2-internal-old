#include "debug.hpp"
#include "client/cs2/convar.hpp"
#include "client/game.hpp"
#include "client/global.hpp"

#include <imgui.h>
#include <vector>
#include <tuple>
#include <Windows.h>
#include <string>
#include <common/logging.hpp>
#include <client/utils.hpp>
#include <client/game.hpp>

#include <client/global.hpp>

static auto schema_dump() -> void {
  if (!ImGui::BeginTabItem("Schema"))
    return;

  ImGui::EndTabItem();
}

static auto convar_dump() -> void {
  if (!ImGui::BeginTabItem("ConVar"))
    return;

  if (ImGui::Button("The funny")) {
    u64 id = 0;
    void * curcvar = *game::intf::convar->iter_get_first(&id);
    int x = 0;
    while (curcvar != (void *)0xFFFFFFFF) {
      cs2log("ConVar: {}", game::intf::convar->get_cvar_from_id(id)->name);
      curcvar = *game::intf::convar->iter_get_next(&id, curcvar);
      ++x;
    }
    cs2log("cvar count: {}", x);
  }

  static char cvarname[256] = {};
  static cs2::convar_data * cvar = nullptr;
  ImGui::InputText("##cs2convarname", cvarname, sizeof(cvarname));
  ImGui::SameLine();
  if (ImGui::Button("Find ConVar")) {
    u64 id = 0;
    game::intf::convar->find_convar(&id, cvarname, 0);
    if (!id || id == 0xFFFF) {
      cs2log("Invalid ConVar");
    } else {
      cvar = game::intf::convar->get_cvar_from_id(id);
      cs2log("Found {} (ID: {:x}) @ {}", cvarname, id, (void *)cvar);
    }
  }

  ImGui::EndTabItem();
}

static auto interface_dump() -> void {
  if (!ImGui::BeginTabItem("Interface"))
    return;

  static char modname[256] = { "client.dll" };
  using interface_entry_t = std::tuple<const char *, void *>;
  using interface_entries_t = std::vector<interface_entry_t>;
  using module_entry_t    = std::pair<std::string, interface_entries_t>;
  static std::vector<module_entry_t> dump;

  ImGui::InputText("Module Target", modname, sizeof(modname));
  ImGui::SameLine();
  if (ImGui::Button("Dump")) {
    HMODULE target = GetModuleHandleA(modname);

    auto & entry = dump.emplace_back(std::make_pair(modname, interface_entries_t {}));
    if (target) {
      for (auto intf : utils::get_module_interfaces(target)) {
        entry.second.emplace_back(std::make_tuple(intf->name, intf->create()));
      }
    } else {
      cs2log("Invalid module name.");
    }

  }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    dump.clear();
  }

  if (ImGui::BeginTable("##cs2interfacedumptable", 3, ImGuiTableFlags_ScrollY, { 0.f, ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - 20.f })) {
    ImGui::TableSetupColumn("Module");
    ImGui::TableSetupColumn("Interface name");
    ImGui::TableSetupColumn("Interface instance");
    ImGui::TableHeadersRow();

    for (auto & [modnm, entries] : dump) {
      for (auto [name, ptr] : entries) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", modnm.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%s", name);
        ImGui::TableNextColumn();
        ImGui::Text("0x%p", ptr);
      }
    } 

    ImGui::EndTable();
  }

  ImGui::EndTabItem();
}

static auto test_features() -> void {
  if (!ImGui::BeginTabItem("Test features"))
    return;

  ImGui::Checkbox("Engine glow (spec)", &global::test::glow);
  ImGui::SameLine();
  ImGui::ColorEdit4("##cs2cglow", global::test::rgb, ImGuiColorEditFlags_NoInputs);

  ImGui::SliderFloat("Force FOV", &global::test::force_fov, -1.f, 180.f);

  ImGui::Checkbox("Force sv_cheats as:", &global::test::force_sv_cheats);
  ImGui::SameLine();
  ImGui::Checkbox("##cs2svcstate", &global::test::force_sv_cheats_state);
  if (ImGui::Button("Thirdperson")) {
    static void(*tpb_cb)(void) = (decltype(tpb_cb))((u8 *)GetModuleHandleA("client.dll") + 0x6BCE30);
    tpb_cb();
  }

  ImGui::EndTabItem();
}

auto menu::tab::debug_tab::on_menu_tab() -> void {
  if (!ImGui::BeginTabBar("##cs2tabdebug"))
    return;

  test_features();
  interface_dump();
  convar_dump();
  schema_dump();

  ImGui::EndTabBar();
}
