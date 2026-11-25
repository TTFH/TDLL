#include <windows.h>
#include <bcrypt.h>

__attribute__((used)) static void* keep_bcrypt = (void*)BCryptGenRandom;

BOOL WINAPI WaitOnAddress(volatile VOID* addr, PVOID cmp, SIZE_T size, DWORD ms) {
	DWORD start = GetTickCount();
	while (TRUE) {
		if (memcmp((const void*)addr, cmp, size) != 0) return TRUE;
		if (ms != INFINITE && GetTickCount() - start >= ms) {
			SetLastError(ERROR_TIMEOUT);
			return FALSE;
		}
		Sleep(1);
	}
}

VOID WINAPI WakeByAddressSingle(PVOID addr) { (void)addr; }

// Compile with:
// gcc -shared -static -o waitaddr_shim.dll stub.c stub.def -lbcrypt
