#pragma once

#if defined(_DEBUG_VERBOSE) 
  #define LOG_PRINT(_f, ...) { char buf[1024]; wsprintfA(buf, _f, ##__VA_ARGS__); OutputDebugStringA(buf); }
#else
  #define LOG_PRINT(_f, ...) {}
#endif

#include "../../base/stdafx.h" 

#include "shii_vars.h"
