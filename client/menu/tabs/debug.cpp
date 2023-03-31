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

  if (ImGui::Button("Dump convar")) {
    u64 id = 0;
    void * curcvar = *game::intf::convar->iter_convar_first(&id);
    int x = 0;
    while (id != cs2::INVALID_CONVAR_ID) {
      cs2log("ConVar: {}", game::intf::convar->get_cvar_from_id(id)->name);
      curcvar = *game::intf::convar->iter_convar_next(&id, curcvar);
      ++x;
    }
    cs2log("cvar count: {}", x);
  }
  ImGui::SameLine();
  if (ImGui::Button("Dump concom")) {
    u64 id = 0;
    void * curcvar = *game::intf::convar->iter_concom_first(&id);
    int x = 0;
    while (id != cs2::INVALID_CONCOM_ID) {
      auto * cc = game::intf::convar->get_concom_from_id(id);
      cs2log("ConCom: {}", cc->name);
      curcvar = *game::intf::convar->iter_concom_next(&id, curcvar);
      ++x;
    }
    cs2log("concom count: {}", x);
  }

  static char cvarname[256] = {};
  static void * cvar = nullptr;
  ImGui::InputText("##cs2convarname", cvarname, sizeof(cvarname));
  ImGui::SameLine();
  if (ImGui::Button("Find ConVar")) {
    u64 id = 0;
    game::intf::convar->find_convar(&id, cvarname, 0);
    if (!id || id == 0xFFFFFFFF) {
      cs2log("Invalid ConVar");
    } else {
      cvar = game::intf::convar->get_cvar_from_id(id);
      cs2log("Found {} (ID: {:x}) @ {}", cvarname, id, (void *)cvar);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Find ConCom")) {
    u64 id = 0;
    game::intf::convar->find_concommand(&id, cvarname);
    if (id == cs2::INVALID_CONCOM_ID) {
      cs2log("Invalid ConCommand");
    } else {
      cvar = game::intf::convar->get_concom_from_id(id);
      auto * xd = (cs2::concom_data *)cvar;
      cs2log("Found {} (ID: {:x}) - Callback: {}", cvarname, id, game::intf::convar->_get_concom_callback(xd));
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

  ImGui::Checkbox("Spoof sv_cheats as:", &global::test::force_sv_cheats);
  ImGui::SameLine();
  ImGui::Checkbox("##cs2svcstate", &global::test::force_sv_cheats_state);
  if (ImGui::Button("Thirdperson")) {
    static void(*tpb_cb)(void) = (decltype(tpb_cb))game::intf::convar->_get_concom_callback(
        game::intf::convar->get_concom_from_id(
          (cs2::concom_id_t)*game::intf::convar->find_concommand((cs2::concom_id_t *)&tpb_cb, "thirdperson")
        )
    );
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
