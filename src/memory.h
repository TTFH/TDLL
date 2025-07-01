#ifndef _MEMORY_H
#define _MEMORY_H

#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <MinHook.h>

template<typename T>
void Patch(T* dst, const T* src) {
	DWORD oldProtect;
	VirtualProtect(dst, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);
	*dst = *src;
	VirtualProtect(dst, sizeof(T), oldProtect, &oldProtect);
}

void PatchBA(BYTE* dst, BYTE* src, unsigned int size);
uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<uintptr_t> offsets);
uintptr_t FindPattern(uintptr_t dwAddress, size_t dwLen, const char* pattern, const char* mask);
uintptr_t FindPatternBis(uintptr_t base, size_t size, const char* pattern);
uintptr_t FindPatternInModule(HMODULE hModule, const char* pattern);

namespace Hook {
	void Init();

	template<typename T>
	void Create(T target, T hook, T* original, const char* target_name = "") {
		if (target == nullptr) {
			printf("Hook::Create target for %s is null\n", target_name);
			return;
		}

		MH_STATUS status;
		status = MH_CreateHook((LPVOID)target, (LPVOID)hook, (LPVOID*)original);
		if (status != MH_OK) {
			printf("MH_CreateHook for %s failed: %s\n", target_name, MH_StatusToString(status));
			return;
		}
		status = MH_EnableHook((LPVOID)target);
		if (status != MH_OK)
			printf("MH_EnableHook for %s failed: %s\n", target_name, MH_StatusToString(status));
	}

	template<typename T>
	void Create(LPCWSTR module, const char* target, T hook, T* original) {
		MH_STATUS status;
		LPVOID target_addr;
		status = MH_CreateHookApiEx(module, target, (LPVOID)hook, (LPVOID*)original, &target_addr);
		if (status != MH_OK) {
			printf("MH_CreateHookApiEx for %s failed: %s\n", target, MH_StatusToString(status));
			return;
		}
		status = MH_EnableHook(target_addr);
		if (status != MH_OK)
			printf("MH_EnableHook for %s failed: %s\n", target, MH_StatusToString(status));
	}
};

#endif
