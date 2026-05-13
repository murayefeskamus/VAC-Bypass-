#pragma once

class c_context {
public:
  void* original_sys_fn    = 0; 
  void* original_module_fn = 0; 
};

CLASS_EXTERN(c_context, ctx);
