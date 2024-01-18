#include <stdio.h>
#include "pros_override.h"
#include "extended_api.h"

#ifdef _MSC_VER
#include "MinHook/include/MinHook.h"
#pragma comment(lib, "MinHook/bin/MinHook.x64.lib")
#else
#include <MinHook.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef BOOL (WINAPI* t_wglSwapBuffers)(HDC hDc);
t_wglSwapBuffers wglSwapBuffersOriginal = nullptr;

typedef void (*t_RegisterGameFunctions) (ScriptCore* core);
t_RegisterGameFunctions td_RegisterGameFunctions = nullptr;

uintptr_t moduleBase;
WNDPROC hGameWindowProc;
bool show_menu = false;
bool awwnb = false;

void Patch(BYTE* dst, BYTE* src, size_t size) {
	DWORD oldProtect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldProtect, &oldProtect);
}

LRESULT CALLBACK WindowProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN && wParam == VK_F1)
		show_menu = !show_menu;
	if (show_menu) {
		CallWindowProc(ImGui_ImplWin32_WndProcHandler, hWnd, uMsg, wParam, lParam);
		//return true; // disable teardown input while menu is open
	}
	return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

void RegisterGameFunctionsHook(ScriptCore* script_core) {
	td_RegisterGameFunctions(script_core);
	const char* script_name = script_core->path.c_str();
	if (strstr(script_name, "DLL") != nullptr) {
		printf("Extending API for script: %s\n", script_name);
		lua_State* L = script_core->state_info->state;
		RegisterLuaCFunctions(L);
	}
	awwnb = false; // reset remove boundary checkbox
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

		ImGuiContext* imguiContext = ImGui::GetCurrentContext();
		printf("DLL ImGuiContext @0x%p\n", (void*)imguiContext);

		ImGuiContext* imgui_ctx_td = (ImGuiContext*)Teardown::GetPointerTo(MEM_OFFSET::ImguiCtx);
		printf("TD  ImGuiContext @0x%p\n", (void*)imgui_ctx_td);
		//ImGui::SetCurrentContext(imgui_ctx_td);
	}

	if (show_menu) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("DLL Utils");
		ImGui::Text("Render distance:");
		static float render_dist = 500;
		ImGui::SliderFloat("##render_dist", &render_dist, 100.0f, 1000.0f, "%.0f");
		if (ImGui::Button("Set render distance")) {
			float* render_addr = (float*)Teardown::GetReferenceTo(MEM_OFFSET::RenderDist);
			Patch((BYTE*)render_addr, (BYTE*)&render_dist, sizeof(float));
		}

		static bool fix_steer = false;
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, fix_steer);
		if (ImGui::Checkbox("Fix GetProperty steer", &fix_steer) && fix_steer) {
			BYTE* get_steer = (BYTE*)Teardown::GetReferenceTo(MEM_OFFSET::GetSteer);
			BYTE steer_fix[] = { 0xF3, 0x0F, 0x10, 0x8F, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xCE, 0xE8, 0x1C, 0x7F, 0x19, 0x00 };
			Patch(get_steer, steer_fix, sizeof(steer_fix));
		}
		ImGui::PopItemFlag();

		if (ImGui::Checkbox("Remove boundaries", &awwnb) && awwnb) {
			Game* game = (Game*)Teardown::GetGame();
			game->scene->boundary.setSize(0);
		}
		ImGui::BeginDisabled();
		static bool telemetry_disabled = !TELEMETRY_ENABLED;
		ImGui::Checkbox("Disable Saber Telemetry", &telemetry_disabled);
		ImGui::EndDisabled();
		ImGui::End();

		//ImGui::ShowDemoWindow();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	return wglSwapBuffersOriginal(hDc);
}

DWORD WINAPI MainThread(LPVOID lpThreadParameter) {
#ifdef DEBUGCONSOLE
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);
	printf("DLL Loaded\n");
#endif
	moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");
	HMODULE openglModule = GetModuleHandleA("opengl32.dll");

	MH_Initialize();
	t_wglSwapBuffers wglSwapBuffers = (t_wglSwapBuffers)GetProcAddress(openglModule, "wglSwapBuffers");
	MH_CreateHook((void*)wglSwapBuffers, (void*)wglSwapBuffersHook, (void**)&wglSwapBuffersOriginal);
	MH_CreateHook((void*)Teardown::GetReferenceTo(MEM_OFFSET::RegisterGameFunctions), (void*)RegisterGameFunctionsHook, (void**)&td_RegisterGameFunctions);
	MH_EnableHook(MH_ALL_HOOKS);

	td_lua_createtable = (t_lua_createtable)Teardown::GetReferenceTo(MEM_OFFSET::LuaCreateTable);
	td_lua_pushstring = (t_lua_pushstring)Teardown::GetReferenceTo(MEM_OFFSET::LuaPushString);
	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
