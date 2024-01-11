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

typedef void (*t_RegisterLuaFunction) (CoreStateInfo* scsi, td_string* function_name, void* function_addr);
t_RegisterLuaFunction td_RegisterLuaFunction = nullptr;

uintptr_t moduleBase;
WNDPROC hGameWindowProc;
bool show_menu = false;

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
		return true;
	}
	return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

bool awwnb = false;
bool registered = false;
bool is_playing = false;

void RegisterLuaCFunctions(lua_State* L) {
/*	luaopen_debug(L);
	luaopen_io(L);
	luaopen_os(L);
	luaopen_package(L);
*/
	lua_pushcfunction(L, GetDllVersion);
	lua_setglobal(L, "GetDllVersion");

	lua_pushcfunction(L, GetWater);
	lua_setglobal(L, "GetWater");
	lua_pushcfunction(L, GetScripts);
	lua_setglobal(L, "GetScripts");
	lua_pushcfunction(L, GetBoundaryVertices);
	lua_setglobal(L, "GetBoundaryVertices");
	lua_pushcfunction(L, GetVehicleWheels);
	lua_setglobal(L, "GetVehicleWheels");
	lua_pushcfunction(L, GetScriptPath);
	lua_setglobal(L, "GetScriptPath");
	lua_pushcfunction(L, GetPlayerFlashlight);
	lua_setglobal(L, "GetPlayerFlashlight");
	lua_pushcfunction(L, GetWaterTransform);
	lua_setglobal(L, "GetWaterTransform");
	lua_pushcfunction(L, GetWaterVertices);
	lua_setglobal(L, "GetWaterVertices");
	lua_pushcfunction(L, GetJointLocalBodyPos);
	lua_setglobal(L, "GetJointLocalBodyPos");
	lua_pushcfunction(L, GetShapeTexture);
	lua_setglobal(L, "GetShapeTexture");
	lua_pushcfunction(L, GetTextureOffset);
	lua_setglobal(L, "GetTextureOffset");
	lua_pushcfunction(L, SetShapeTexture);
	lua_setglobal(L, "SetShapeTexture");
	lua_pushcfunction(L, SetTextureOffset);
	lua_setglobal(L, "SetTextureOffset");
	lua_pushcfunction(L, ZlibSaveCompressed);
	lua_setglobal(L, "ZlibSaveCompressed");
	lua_pushcfunction(L, ZlibLoadCompressed);
	lua_setglobal(L, "ZlibLoadCompressed");
}

void RegisterGameFunctionsHook(ScriptCore* core) {
	td_RegisterGameFunctions(core);
	const char* script_name = core->path.c_str();
	if (strstr(script_name, "DLL") != NULL) {
		printf("Extending API for script: %s\n", script_name);
		lua_State* L = core->core_state_info.state_info->state; // TODO: rename to something that make sense
		RegisterLuaCFunctions(L);
	}
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
		printf("DLL ImGuiContext %p\n", (void*)imguiContext);

		ImGuiContext* imgui_ctx_td = (ImGuiContext*)Teardown::GetPointerTo(MEM_OFFSET::ImguiCtx);
		printf("TD  ImGuiContext %p\n", (void*)imgui_ctx_td);
		//ImGui::SetCurrentContext(imgui_ctx_td);
	}

	if (show_menu) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		static bool enabled = true;
		static bool disabled = false;
		static bool telemetry_disabled = !TELEMETRY_ENABLED;

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
			game->scene->boundary.removeVertices();
		}
		ImGui::BeginDisabled();
		ImGui::Checkbox("Show Dev Menu", &disabled);
		ImGui::Checkbox("Disable SV Clamping", &disabled);
		ImGui::Checkbox("Disable Saber Telemetry", &telemetry_disabled);
		ImGui::EndDisabled();
		if (ImGui::Button("RESET")) {
			awwnb = false;
			registered = false;
		}
		ImGui::End();


		ImGui::Begin("Extra Lua libs");
		ImGui::Text("Enabled libraries:");
		static bool libs_enabled = false;
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Checkbox("coroutine", &enabled);
		ImGui::Checkbox("debug", &libs_enabled);
		ImGui::Checkbox("io", &libs_enabled);
		ImGui::Checkbox("math", &enabled);
		ImGui::Checkbox("os", &libs_enabled);
		ImGui::Checkbox("package", &libs_enabled);
		ImGui::Checkbox("string", &enabled);
		ImGui::Checkbox("table", &enabled);
		ImGui::PopItemFlag();
		if (ImGui::Button("Enable All") && !libs_enabled) {
			libs_enabled = true;
		}
		ImGui::End();


		ImGui::Begin("Extended Teardown API");
		ImGui::Text("Enabled functions:");
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Checkbox("[h] = GetWater()", &registered);
		ImGui::Checkbox("[h] = GetScripts()", &registered);
		ImGui::Checkbox("[p] = GetBoundaryVertices()", &registered);
		ImGui::Checkbox("[h] = GetVehicleWheels(v)", &registered);
		ImGui::Checkbox("path = GetScriptPath(h)", &registered);
		ImGui::Checkbox("h = GetPlayerFlashlight()", &registered);
		ImGui::Checkbox("tr = GetWaterTransform(w)", &registered);
		ImGui::Checkbox("[p] = GetWaterVertices(w)", &registered);
		ImGui::Checkbox("p1, p2 = GetJointLocalBodyPos(j)", &registered);
		ImGui::Checkbox("t, w, bt, bw = GetShapeTexture(s)", &registered);
		ImGui::Checkbox("x, y, z = GetTextureOffset(s)", &registered);
		ImGui::Dummy(ImVec2(0, 5));
		ImGui::Checkbox("SetShapeTexture(s, t, w, bt, bw)", &registered);
		ImGui::Checkbox("SetTextureOffset(s, x, y, z)", &registered);
		ImGui::Dummy(ImVec2(0, 5));
		ImGui::Checkbox("ZlibSaveCompressed(file, str);", &registered);
		ImGui::Checkbox("str = ZlibLoadCompressed(file);", &registered);

		ImGui::PopItemFlag();
		if (ImGui::Button("Register All") && !registered) {
			registered = true;
		}
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
#ifdef _MSC_VER
	moduleBase = (uintptr_t)GetModuleHandle(L"teardown.exe");
	HMODULE openglModule = GetModuleHandle(L"opengl32.dll");
#else
	moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");
	HMODULE openglModule = GetModuleHandleA("opengl32.dll");
#endif

	MH_Initialize();
	void* wglSwapBuffers = (void*)GetProcAddress(openglModule, "wglSwapBuffers");
	MH_CreateHook(wglSwapBuffers, (void*)wglSwapBuffersHook, (void**)&wglSwapBuffersOriginal);
	MH_CreateHook((LPVOID)Teardown::GetReferenceTo(MEM_OFFSET::RegisterGameFunctions), (void*)RegisterGameFunctionsHook, (void**)&td_RegisterGameFunctions);
	MH_EnableHook(MH_ALL_HOOKS);

	//td_RegisterLuaFunction = (t_RegisterLuaFunction)Teardown::GetReferenceTo(MEM_OFFSET::RegisterLuaFunction);
	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
