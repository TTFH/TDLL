#include <string.h>

#include "memory.h"

#define PSAPI_VERSION 2
#include <psapi.h>

void PatchBA(BYTE* dst, BYTE* src, unsigned int size) {
	DWORD oldProtect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldProtect, &oldProtect);
}

uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<uintptr_t> offsets) {
	uintptr_t addr = ptr;
	for (size_t i = 0; i < offsets.size(); i++) {
		addr = *(uintptr_t*)addr;
		if (IsBadReadPtr((uintptr_t*)addr, sizeof(addr)))
			return 0;
		addr += offsets[i];
	}
	return addr;
}

uintptr_t FindPattern(uintptr_t dwAddress, size_t dwLen, const char* pattern, const char* mask) {
	for (size_t i = 0; i < dwLen - strlen(mask); i++) {
		bool found = true;
		for (size_t j = 0; j < strlen(mask); j++) {
			if (mask[j] != '?' && pattern[j] != *(char*)(dwAddress + i + j)) {
				found = false;
				break;
			}
		}
		if (found) return i;
	}
	return 0;
}

uintptr_t FindPatternInModule(HMODULE hModule, const char* pattern, const char* mask) {
	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO));
	return FindPattern((uintptr_t)hModule, (size_t)moduleInfo.SizeOfImage, pattern, mask);
}

namespace Hook {
	void Init() {
		MH_Initialize();
	}
}
