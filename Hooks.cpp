#include "stdafx.h"


EXPORT bool __stdcall module_fucking_handler(void* instance, u32 reason, void* reserved){
  prepare_internal_ctx();

  VM_MUTATE_START()
  if(true){ 
    static bool is_module_ready = false;
    uptr service_base = utils::get_module_handle(HASH("steamservice.dll"));

    if(!is_module_ready && service_base != 0){
      update_pipe_state(XOR32(HASH("STATE_APPLYING_PATCH")));

      uptr target_instr_addr = utils::find_signature(service_base, SIG("\x74\x47\x6A\x01\x6A"), XOR32(0xA0000));

      if(target_instr_addr != 0){
        u32 old_protect;
        VirtualProtect(target_instr_addr, XOR32(1), PAGE_EXECUTE_READWRITE, &old_protect);
        *(u8*)target_instr_addr = XOR32(0xEB); 
        VirtualProtect(target_instr_addr, XOR32(1), old_protect, &old_protect);

        is_module_ready = true;
        file_map->create(HASH("STATE_SUCCESS_VERIFIED"));
      }
    }
  }
  VM_MUTATE_STOP()

  return utils::call_stdcall<bool, void*, u32, void*>(gen_internal->decrypt_asset(ctx->original_module_fn), instance, reason, reserved);
}



/* =============================================================================================
   alternatif 2. bir yontem: IAT (Import Address Table) Hooking ve Module Takibi

#include <Windows.h>


HMODULE WINAPI Hooks_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE result = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

    Utils_hookImport(lpLibFileName, "kernel32.dll", "GetProcAddress", Hooks_GetProcAddress);
    Utils_hookImport(lpLibFileName, "kernel32.dll", "GetSystemInfo", Hooks_GetSystemInfo);

    return result;
}

HMODULE WINAPI Hooks_LoadLibraryExW_SteamClient(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE result = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

    if (wcsstr(lpLibFileName, L"steamui.dll")) {
        Utils_hookImport(lpLibFileName, "kernel32.dll", "LoadLibraryExW", Hooks_LoadLibraryExW_SteamClient);
    } else if (wcsstr(lpLibFileName, L"steamservice.dll")) {
        PBYTE toPatch = Utils_findPattern(L"steamservice", "\x74\x47\x6A\x01\x6A", 0);
        if (toPatch) {
            DWORD old;
            VirtualProtect(toPatch, 1, PAGE_EXECUTE_READWRITE, &old);
            *toPatch = 0xEB;
            VirtualProtect(toPatch, 1, old, &old);
            
            Utils_hookImport(L"steamservice", "kernel32.dll", "LoadLibraryExW", Hooks_LoadLibraryExW);
        }
    }
    return result;
}

FARPROC WINAPI Hooks_GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    FARPROC result = GetProcAddress(hModule, lpProcName);
 
    if (result) {
        if (!strcmp(lpProcName, "GetProcAddress"))
            return (FARPROC)Hooks_GetProcAddress;
        else if (!strcmp(lpProcName, "GetSystemInfo"))
            return (FARPROC)Hooks_GetSystemInfo;
        else if (!strcmp(lpProcName, "GetVersionExA"))
            return (FARPROC)Hooks_GetVersionExA;
    }
    return result;
}

VOID WINAPI Hooks_GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
    GetSystemInfo(lpSystemInfo);
    lpSystemInfo->dwPageSize = 1337; 
}

BOOL WINAPI Hooks_GetVersionExA(LPOSVERSIONINFOEXA lpVersionInformation) {
    ExitProcess(1);
    return FALSE;
}

=============================================================================================
*/





static void update_pipe_state(u32 c){
  static u32* signal_buf = (u32*)malloc(sizeof(u32));
  if(signal_buf == nullptr){
    send_status_code(HASH("CORE_BYPASS"), c);
    return;
  }

  static bool is_busy = false;
  *signal_buf = c;
  utils::create_worker([](void* ptr){
    u32 code = *(u32*)ptr;
    while(is_busy) I(Sleep)(50);
    is_busy = true;
    send_status_code(XOR32(HASH("CORE_BYPASS")), code);
    is_busy = false;
    return 0;
  }, signal_buf);
}

#define REPORT_STATE(x) update_pipe_state(XOR32(HASH(x)));

EXPORT void __fastcall sys_query_callback_shii(SYSTEM_INFO* info, void* rdx){
  prepare_internal_ctx();

  utils::call_fastcall64<void, SYSTEM_INFO*>(gen_internal->decrypt_asset(ctx->original_sys_fn), info, rdx);

  if(info != nullptr)
    info->dwPageSize = 1; 

  static bool reported = false;
  if(!reported){
    reported = true;
    REPORT_STATE("STATE_SYS_INFO_SUCCESS");
  }
}

bool prepare_internal_ctx(){
  static bool initialized = false;
  if(initialized)
    return true;

  REPORT_STATE("STATE_INIT_START");

  {
    ctx->original_sys_fn = gen_internal->get_pkg(HASH("sys_query_callback_shii"));
    ctx->original_module_fn = gen_internal->get_pkg(HASH("module_fucking_handler"));
  }

  gen_internal->loaded = true;
  initialized = true;
  return true;
}
