#include <Windows.h>
#include <TlHelp32.h>
#include <handleapi.h>
#include <metapp/metapp.hpp>
#include <filesystem>
#include <common/logging.hpp>
#include <minwinbase.h>
#include <string_view>
#include <synchapi.h>
#include <winnt.h>

auto main(int argc, char ** argv) -> int {
  if (!std::filesystem::exists(CS2INT_TARGET_LIB)) {
    cs2log("Target library not found ({})", CS2INT_TARGET_LIB);
    return 1;
  }

  HANDLE psnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
  if (psnap == INVALID_HANDLE_VALUE) {
    cs2log("Failed to create process snapshot.");
    return 1;
  }
  mpp_defer { CloseHandle(psnap); };

  DWORD  pid   = -1;
  HANDLE hproc = NULL;
  PROCESSENTRY32 pe32 = { .dwSize = sizeof(pe32), };
  if (Process32First(psnap, &pe32)) {
    do {
      if (std::string_view(pe32.szExeFile) != "cs2.exe")
        continue; 
      pid = pe32.th32ProcessID;
      hproc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
      break;
    } while(Process32Next(psnap, &pe32));
  }

  if (!hproc || hproc == INVALID_HANDLE_VALUE || pid == -1) {
    cs2log("Failed to open process.");
    return 1;
  }
  mpp_defer { CloseHandle(hproc); };

  HANDLE msnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
  if (!msnap || msnap == INVALID_HANDLE_VALUE) {
    cs2log("Failed to create module snapshot.");
    return 1;
  }
  mpp_defer { CloseHandle(msnap); };
  
  MODULEENTRY32 me32 = { .dwSize = sizeof(me32), };
  if (Module32First(msnap, &me32)) {
    do {
      if (std::string_view(me32.szModule) == CS2INT_TARGET_FNAME) {
        cs2log("Client is already loaded! Running unload routine...");
        HANDLE hr_unthread = CreateRemoteThread(hproc, NULL, NULL, LPTHREAD_START_ROUTINE(&FreeLibrary), me32.hModule, NULL, NULL);
        if (!hr_unthread || hr_unthread == INVALID_HANDLE_VALUE) {
          cs2log("Failed to create remote thread for unloading.");
          return 1;
        }
        WaitForSingleObject(hr_unthread, INFINITE);
        cs2log("Unloaded!");
        return 0;
      }
    } while (Module32Next(msnap, &me32));
  }

  LPVOID pr_path = VirtualAllocEx(hproc, NULL, sizeof(CS2INT_TARGET_LIB), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!pr_path) {
    cs2log("Failed to allocate memory.");
    return 1;
  }
  mpp_defer { VirtualFreeEx(hproc, pr_path, NULL, MEM_RELEASE); };

  if (!WriteProcessMemory(hproc, pr_path, CS2INT_TARGET_LIB, sizeof(CS2INT_TARGET_LIB), NULL)) {
    cs2log("Failed to write path.");
    return 1;
  }

  HANDLE hr_thread = CreateRemoteThread(hproc, NULL, NULL, LPTHREAD_START_ROUTINE(&LoadLibraryA), pr_path, NULL, NULL);
  if (!hr_thread || hr_thread == INVALID_HANDLE_VALUE) {
    cs2log("Failed to create remote thread for loading.");
    return 1;
  }
  mpp_defer { CloseHandle(hr_thread); };
  WaitForSingleObject(hr_thread, INFINITE);
  cs2log("Loaded!");

  return 0;
}
