#pragma once

namespace cs2 {

class iinputservice {
public:
  // [03/04/2023] find this by looking for the string "toggle imgui_enable" in client.dll
  inline auto client_cmd(int maybe_index, const char * cmd, int unk) -> void {
    return reinterpret_cast<void(***)(void *, int, const char *, int)>(this)[0][25](this, maybe_index, cmd, unk);
  }
};

} // cs2
