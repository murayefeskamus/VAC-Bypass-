#pragma once

class c_context {
public:
  void* original_sys_fn    = 0; // Eski adıyla: get_system_info_hook_trp
  void* original_module_fn = 0; // Eski adıyla: dll_main_hook_trp
};

CLASS_EXTERN(c_context, ctx);
