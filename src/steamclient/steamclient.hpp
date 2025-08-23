#pragma once

#define C_DECL(TYPE) extern "C" __declspec(noinline) TYPE __cdecl

C_DECL(void*) CreateInterface(const char* interface_string, int* out_result);
