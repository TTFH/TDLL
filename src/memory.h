#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <MinHook.h>

template<typename T>
void Patch(T* dst, const T* src) {
	DWORD oldProtect;
	VirtualProtect(dst, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);
	*dst = *src;
	VirtualProtect(dst, sizeof(T), oldProtect, &oldProtect);
}

uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<uintptr_t> offsets);
uintptr_t FindPattern(uintptr_t dwAddress, size_t dwLen, const char* pattern, const char* mask);
uintptr_t FindPatternInModule(HMODULE hModule, const char* pattern, const char* mask);

namespace Hook {
	void Init();
	template<typename T>
	void Create(T target, T hook, T* original) {
		MH_CreateHook((LPVOID)target, (LPVOID)hook, (LPVOID*)original);
		MH_EnableHook((LPVOID)target);
	}
	template<typename T>
	void Create(LPCWSTR module, const char* target, T hook, T* original) {
		LPVOID target_addr;
		MH_CreateHookApiEx(module, target, (LPVOID)hook, (LPVOID*)original, &target_addr);
		MH_EnableHook(target_addr);
	}
};

#endif
