#include "debug.hpp"
#include "client/cs2/convar.hpp"
#include "client/cs2/vecang.hpp"
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

  static char cvarname[256] = {};
  static void * cvar = nullptr;
  ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.35);
  ImGui::InputText("##cs2convarname", cvarname, sizeof(cvarname));
  ImGui::SameLine();
  if (ImGui::Button("Find ConVar")) {
    u64 id = 0;
    game::intf::convar->find_convar(&id, cvarname, 0);
    if (!id || id == 0xFFFFFFFF) {
      cs2log("Invalid ConVar");
    } else {
      cvar = game::intf::convar->get_convar_from_id(id);
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
      cs2log("Found {} (ID: {:x}) - Callback: {}", cvarname, id, (void *)utils::find_concom_callback_str(cvarname));
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Find ConVar manually")) {
    auto * con = utils::find_convar_str(cvarname);
    if (!con) {
      cs2log("Invalid ConVar entry");
    } else {
      cs2log("Found {} @ {}", cvarname, (void *)con);
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Find ConCom manually")) {
    auto * con = utils::find_concom_str(cvarname);
    if (!con) {
      cs2log("Invalid ConVar entry");
    } else {
      cs2log("Found {} @ {}", cvarname, (void *)con);
    }
  }

  if (ImGui::Button("Dump convar")) {
    u64 id = 0;
    auto curcvar = *game::intf::convar->iter_convar_first(&id);
    int x = 0;
    while (id != cs2::INVALID_CONVAR_ID) {
      cs2log("ConVar: {}", game::intf::convar->get_convar_from_id(id)->name);
      curcvar = *game::intf::convar->iter_convar_next(&id, curcvar);
      ++x;
    }
    cs2log("cvar count: {}", x);
  }
  ImGui::SameLine();
  if (ImGui::Button("Dump concom")) {
    u64 id = 0;
    auto curcvar = *game::intf::convar->iter_concom_first(&id);
    int x = 0;
    while (id != cs2::INVALID_CONCOM_ID) {
      auto * cc = game::intf::convar->get_concom_from_id(id);
      cs2log("ConCom: {}", cc->name);
      curcvar = *game::intf::convar->iter_concom_next(&id, curcvar);
      ++x;
    }
    cs2log("concom count: {}", x);
  }
  ImGui::SameLine();
  if (ImGui::Button("Manual dump convar")) {
    for (int i = 0; i < game::intf::convar->convar_count; ++i) {
      auto & entry = game::intf::convar->convar_entries[i];
      if (entry.data)
        cs2log("MConVar dump: {} @ {}", entry.data->name, (void *)entry.data);
      else
        cs2log("MConVar dump at index {} is empty.", i);
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Manual dump concom")) {
    for (int i = 0; i < game::intf::convar->concom_count; ++i) {
      auto & entry = game::intf::convar->concom_entries[i];
      cs2log("MConCom dump: {} @ {}", entry.name, (void *)&entry);
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Clear##cs2condump")) {
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

static bool show_localplayer_test_window = true;
static auto test_features() -> void {
  if (!ImGui::BeginTabItem("Test features"))
    return;

  if (global::obs_present_ptr)
    ImGui::Checkbox("OBS Presence", &global::obs_presence);
  ImGui::Checkbox("Engine glow (spec)", &global::test::glow);
  ImGui::SameLine();
  ImGui::ColorEdit4("##cs2cglow", global::test::rgb, ImGuiColorEditFlags_NoInputs);

  ImGui::SliderFloat("Force FOV", &global::test::force_fov, -1.f, 180.f);

  ImGui::Checkbox("Spoof sv_cheats as:", &global::test::force_sv_cheats);
  ImGui::SameLine();
  ImGui::Checkbox("##cs2svcstate", &global::test::force_sv_cheats_state);
  if (ImGui::Button("Thirdperson")) {
    game::concom::thirdperson();
  }

  static char cmd[256] = {};
  ImGui::InputText("##cs2cmd", cmd, sizeof(cmd));
  ImGui::SameLine();
  if (ImGui::Button("Execute Command##cs2")) {
    utils::cmd(cmd);
  }

  ImGui::Checkbox("No smoke", &global::test::no_smoke);
  ImGui::Checkbox("Debug Localplayer info test window thing", &show_localplayer_test_window);

  ImGui::EndTabItem();
}

auto menu::tab::debug_tab::on_imgui() -> void {
  if (!show_localplayer_test_window)
    return;

  auto ent = game::client_get_entity_by_index(0);
  if (!ent)
    return;
  cs2::vec3 pos;
  cs2::ang3 ang;
  ent->get_eyepos(&pos);
  ent->get_viewangles(&ang);
  ImGui::Begin("Local Entity test");
  ImGui::Text("EyePos: %f, %f, %f", pos.x, pos.y, pos.z);
  ImGui::Text("Angle: %f, %f, %f", ang.pitch, ang.yaw, ang.roll);
  ImGui::End();
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
