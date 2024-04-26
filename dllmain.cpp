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
t_wglSwapBuffers td_wglSwapBuffers = nullptr;

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
		return true; // disable teardown input while menu is open
	}
	return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

void RegisterGameFunctionsHook(ScriptCore* script_core) {
	td_RegisterGameFunctions(script_core);
	const char* script_name = script_core->path.c_str();
	//if (strstr(script_name, "DLL") != nullptr) {
		printf("Extending API for script: %s\n", script_name);
		lua_State* L = script_core->state_info->state;
		RegisterLuaCFunctions(L);
	//}
	awwnb = false; // reset remove boundary checkbox
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
			Patch((BYTE*)render_addr, (BYTE*)&render_dist, sizeof(float));
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

DWORD WINAPI MainThread(LPVOID lpThreadParameter) {
#ifdef DEBUGCONSOLE
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
	printf("DLL Loaded\n");
#endif
	/*bool init = false;
	while (!init) {
		if (GetAsyncKeyState(VK_F1) & 1) {
			init = true;
			break;
		}
		Sleep(100);
	}*/
	Sleep(5000); // wait for Steam DRM to load the game

	if (MH_Initialize() != MH_OK)
		printf("Failed to initialize MinHook\n");

	moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");
	//HMODULE openglModule = GetModuleHandleA("opengl32.dll");
	//t_wglSwapBuffers wglSwapBuffers = (t_wglSwapBuffers)GetProcAddress(openglModule, "wglSwapBuffers");
	//MH_CreateHook((void*)wglSwapBuffers, (void*)wglSwapBuffersHook, (void**)&td_wglSwapBuffers);

	LPVOID target = nullptr;
	if (MH_CreateHookApiEx(L"opengl32.dll", "wglSwapBuffers", (void*)wglSwapBuffersHook, (void**)&td_wglSwapBuffers, &target) != MH_OK)
		printf("Failed to create hook for wglSwapBuffers\n");
	uintptr_t rgf_addr = Teardown::GetReferenceTo(MEM_OFFSET::RegisterGameFunctions);
	MH_STATUS status = MH_CreateHook((void*)rgf_addr, (void*)RegisterGameFunctionsHook, (void**)&td_RegisterGameFunctions);
	if (status != MH_OK) {
		printf("Failed to create hook for RegisterGameFunctions\n");
		printf("Error code: %d\n", status);
	}
	if (MH_EnableHook((void*)target) != MH_OK)
		printf("Failed to enable hook for wglSwapBuffers\n");
	if (MH_EnableHook((void*)rgf_addr) != MH_OK)
		printf("Failed to enable hook for RegisterGameFunctions\n");

	td_lua_createtable = (t_lua_createtable)Teardown::GetReferenceTo(MEM_OFFSET::LuaCreateTable);
	td_lua_pushstring = (t_lua_pushstring)Teardown::GetReferenceTo(MEM_OFFSET::LuaPushString);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
