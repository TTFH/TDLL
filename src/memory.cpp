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
		if (found) return dwAddress + i;
	}
	return 0;
}

// Expend 5 minutes to write a function that works
// or
// Ask ChatGPT to write it and spend 2 days debugging why it doesn't work, and draw 25
uintptr_t FindPatternBis(uintptr_t base, size_t size, const char* pattern) {
	uint8_t* data = reinterpret_cast<uint8_t*>(base);

	auto parseByte = [](const char* str, uint8_t& byte, bool& isWildcard) -> size_t {
		size_t advance = 0;
		isWildcard = false;
		char ch = *str;
		if (ch == '?') {
			isWildcard = true;
			advance = (str[1] == '?') ? 2 : 1;
		} else if (isxdigit(ch)) {
			char byteStr[3] = { ch, str[1], '\0' };
			byte = static_cast<uint8_t>(strtol(byteStr, nullptr, 16));
			advance = 2;
		} else {
			printf("Invalid pattern char: %c\n", ch);
			exit(EXIT_FAILURE);
		}
		return advance;
	};

	size_t pattern_length = strlen(pattern);
	for (size_t i = 0; i < size - pattern_length; i++) {
		const char* pat = pattern;
		size_t j = 0;
		while (*pat != '\0' && i + j < size) {
			while (isspace(*pat)) pat++;
			if (*pat == '\0') break;
			uint8_t pat_byte = 0;
			bool wildcard = false;
			size_t advance = parseByte(pat, pat_byte, wildcard);
			if (!wildcard && data[i + j] != pat_byte)
				break;
			pat += advance;
			j++;
		}
		if (*pat == '\0')
			return base + i;
	}
	return 0;
}

uintptr_t FindPatternInModule(HMODULE hModule, const char* pattern) {
	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO));
	return FindPatternBis((uintptr_t)hModule, (size_t)moduleInfo.SizeOfImage, pattern);
}

namespace Hook {
	void Init() {
		MH_Initialize();
	}
}
