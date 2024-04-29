#include <stdio.h>

#include "extended_api.h"
#include "pros_override.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#ifdef _MSC_VER
#include "MinHook/include/MinHook.h"
#pragma comment(lib, "MinHook/bin/MinHook.x64.lib")
#else
#include <MinHook.h>
#endif

#define PSAPI_VERSION 2
#include <psapi.h>

typedef BOOL WINAPI (*t_wglSwapBuffers) (HDC hDc);
t_wglSwapBuffers td_wglSwapBuffers = nullptr;

typedef void (*t_RegisterGameFunctions) (ScriptCore* core);
t_RegisterGameFunctions td_RegisterGameFunctions = nullptr;

typedef void (*LuaCFunctionEx) (ScriptCore* core, lua_State*& L, ReturnInfo* ret);
typedef void (*t_RegisterLuaFunction) (ScriptCoreInner* inner_core, td_string* func_name, LuaCFunctionEx func_addr);
t_RegisterLuaFunction td_RegisterLuaFunction = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HMODULE moduleBase;
WNDPROC hGameWindowProc;
bool show_menu = false;
bool awwnb = false;

template<typename T>
void Patch(T* dst, const T* src) {
	DWORD oldProtect;
	VirtualProtect(dst, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);
	*dst = *src;
	VirtualProtect(dst, sizeof(T), oldProtect, &oldProtect);
}

LRESULT CALLBACK WindowProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN && wParam == VK_F1)
		show_menu = !show_menu;
	if (show_menu) {
		CallWindowProc(ImGui_ImplWin32_WndProcHandler, hWnd, uMsg, wParam, lParam);
		return true; // Disable teardown input while menu is open
	}
	return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

void Print(ScriptCore* core, lua_State*& L, ReturnInfo* ret) {
	const char* text = lua_tostring(L, 1);
	lua_getglobal(L, "DebugPrint");
	lua_pushstring(L, text);
	lua_call(L, 1, 0);
}

void UiGetAlign(ScriptCore* core, lua_State*& L, ReturnInfo* ret) {
	static const char* H_ALIGNS[] = { "left", "center", "right" };
	static const char* V_ALIGNS[] = { "top", "center", "bottom" };
	char alignment[16];
	int horizontal = core->ui_status.align_h;
	int vertical = core->ui_status.align_v;
	sprintf(alignment, "%s %s", H_ALIGNS[horizontal], V_ALIGNS[vertical]);
	lua_pushstring(L, alignment);
	ret->ret_count = 1;
}

void RegisterGameFunctionsHook(ScriptCore* script_core) {
	td_RegisterGameFunctions(script_core);
	//const char* script_name = script_core->path.c_str();
	lua_State* L = script_core->inner_core.state_info->state;
	RegisterLuaCFunctions(L);
/*
	td_string print("Print");
	td_string ui_get_align("UiGetAlign");
	td_RegisterLuaFunction(&script_core->inner_core, &print, Print);
	td_RegisterLuaFunction(&script_core->inner_core, &ui_get_align, UiGetAlign);
*/
	awwnb = false; // Reset remove boundary checkbox
	for (int i = 0; i < 16; i++)
		clock_init[i] = false;
}

BOOL wglSwapBuffersHook(HDC hDc) {
	static bool imGuiInitialized = false;
	if (!imGuiInitialized) {
		imGuiInitialized = true;
		HWND hGameWindow = WindowFromDC(hDc);
		hGameWindowProc = (WNDPROC)SetWindowLongPtr(hGameWindow, GWLP_WNDPROC, (LONG_PTR)WindowProcHook);

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
		ImGui_ImplWin32_Init(hGameWindow);
		ImGui_ImplOpenGL3_Init();
	}

	if (show_menu) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("DLL Utils");
		ImGui::Text("Render distance:");
		static float render_dist = 500;
		ImGui::SliderFloat("##render_dist", &render_dist, 100.0f, 1000.0f, "%.0f");
		ImGui::SameLine();
		ImGui::InputFloat("##render_dist_input", &render_dist, 1.0f, 10.0f, "%.0f");
		if (ImGui::Button("Set render distance")) {
			float* render_addr = (float*)Teardown::GetReferenceTo(MEM_OFFSET::RenderDist);
			Patch(render_addr, &render_dist);
		}
		if (ImGui::Checkbox("Remove boundaries", &awwnb) && awwnb) {
			Game* game = (Game*)Teardown::GetGame();
			game->scene->boundary.setSize(0);
		}
		ImGui::BeginDisabled();
		static bool telemetry_disabled = !TELEMETRY_ENABLED;
		ImGui::Checkbox("Disable Saber Telemetry", &telemetry_disabled);
		ImGui::EndDisabled();
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	return td_wglSwapBuffers(hDc);
}

class Hook {
public:
	Hook() {
		MH_Initialize();
	}
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

DWORD WINAPI MainThread(LPVOID lpThreadParameter) {
	Sleep(5000);
#ifdef DEBUGCONSOLE
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("TDLL Loaded\n");
#endif

	Hook hook;
	moduleBase = GetModuleHandleA("teardown.exe");
	td_lua_pushstring = (t_lua_pushstring)Teardown::GetReferenceTo(MEM_OFFSET::LuaPushString);
	td_lua_createtable = (t_lua_createtable)Teardown::GetReferenceTo(MEM_OFFSET::LuaCreateTable);
	td_RegisterLuaFunction = (t_RegisterLuaFunction)Teardown::GetReferenceTo(MEM_OFFSET::RegisterLuaFunction);
	t_RegisterGameFunctions rgf_addr = (t_RegisterGameFunctions)Teardown::GetReferenceTo(MEM_OFFSET::RegisterGameFunctions);
	hook.Create(L"opengl32.dll", "wglSwapBuffers", wglSwapBuffersHook, &td_wglSwapBuffers);
	hook.Create(rgf_addr, RegisterGameFunctionsHook, &td_RegisterGameFunctions);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
