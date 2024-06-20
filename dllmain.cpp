#include <time.h>
#include <stdio.h>

#include "glad/glad.h"
//#include <GLFW/glfw3.h>

#include "extended_api.h"
#include "pros_override.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
//#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#ifdef _MSC_VER
#include "MinHook/include/MinHook.h"
#pragma comment(lib, "MinHook/bin/MinHook.x64.lib")
#else
#include <MinHook.h>
#endif

#define PSAPI_VERSION 2
#include <psapi.h> // Used to get .exe size

t_lua_createtable td_lua_createtable = nullptr;
t_lua_pushstring td_lua_pushstring = nullptr;

typedef BOOL WINAPI (*t_wglSwapBuffers) (HDC hDc);
t_wglSwapBuffers td_wglSwapBuffers = nullptr;

typedef void (*t_RegisterGameFunctions) (ScriptCore* core);
t_RegisterGameFunctions td_RegisterGameFunctions = nullptr;

typedef void (*t_ScreenCaptureThread) (CaptureThread* ct);
t_ScreenCaptureThread td_ScreenCaptureThread = nullptr;

typedef void (*t_mutex) (void* mutex);
t_mutex td_mtx_lock_wrapper = nullptr;
t_mutex td_mtx_unlock = nullptr;

typedef void (*LuaCFunctionEx) (ScriptCore* core, lua_State* &L, ReturnInfo* ret);
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

void UiGetAlign(ScriptCore* core, lua_State* &L, ReturnInfo* ret) {
	static const char* V_ALIGNS[] = { "bottom", "top", "middle" };
	static const char* H_ALIGNS[] = { "right", "left", "center" };
	char alignment[32];
	int vertical = core->ui_status->align_v % 3;
	int horizontal = core->ui_status->align_h % 3;
	sprintf(alignment, "%s %s", V_ALIGNS[vertical], H_ALIGNS[horizontal]);
	td_lua_pushstring(L, alignment);
	ret->count = 1;
}

void UiPlayVideo(ScriptCore* core, lua_State* &L, ReturnInfo* ret) {
	// TODO: Implement video playback
	/*
	local frame = GetTime() * FPS
	frame = frame % FRAMES + 1
	local path = string.format("MOD/frames/%04d.png", frame)
	-- TODO: play audio
	*/
	const char* path = lua_tostring(L, 1);
	lua_getglobal(L, "UiImage");
	td_lua_pushstring(L, path);
	lua_call(L, 1, 0);
}

void RegisterLuaFunctionHook(ScriptCoreInner* inner_core, td_string* func_name, LuaCFunctionEx func_addr) {
	lua_State* L = inner_core->state_info->state;
	printf("%p | Function: %s\n", (void*)L, func_name->c_str());
	td_RegisterLuaFunction(inner_core, func_name, func_addr);
}

void RegisterLuaFunctionProxy(ScriptCore* core, const char* func_name, LuaCFunctionEx func_addr) {
	td_string name(func_name);
	td_RegisterLuaFunction(&core->inner_core, &name, func_addr);
}

void RegisterGameFunctionsHook(ScriptCore* core) {
	td_RegisterGameFunctions(core);
	lua_State* L = core->inner_core.state_info->state;
	//const char* script_name = core->path.c_str();
	//printf("%p | Script: %s\n", (void*)L, script_name);

	RegisterLuaCFunctions(L);
	RegisterLuaFunctionProxy(core, "UiGetAlign", UiGetAlign);

	// TODO: improve, this may break if a script is spawned
	awwnb = false; // Reset remove boundary checkbox
	for (int i = 0; i < 16; i++)
		clock_init[i] = false;
}

void FlipImageVertically(int width, int height, uint8_t* data) {
	uint8_t rgb[3];
	for (int y = 0; y < height / 2; y++) {
		for (int x = 0; x < width; x++) {
			int top = 3 * (x + y * width);
			int bottom = 3 * (x + (height - y - 1) * width);
			memcpy(rgb, data + top, sizeof(rgb));
			memcpy(data + top, data + bottom, sizeof(rgb));
			memcpy(data + bottom, rgb, sizeof(rgb));
		}
	}
}

void Screenshot(int width, int height) {
	uint8_t* pixels = new uint8_t[width * height * 3];
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	FlipImageVertically(width, height, pixels);
	std::string filename = "./screenshot_" + std::to_string(time(NULL)) + ".png";
	stbi_write_png(filename.c_str(), width, height, 3, pixels, 3 * width);
	delete[] pixels;
}

void SaveImageJPG(const char* path, int width, int height, uint8_t* pixels) {
	FlipImageVertically(width, height, pixels);
	stbi_write_jpg(path, width, height, 3, pixels, 100);
}

void ScreenCaptureThreadHook(CaptureThread* ct) {
	printf("%dx%d %s\n", ct->width, ct->height, ct->image_path.c_str());
	td_ScreenCaptureThread(ct);
	/*while (!ct->done) {
		if (ct->save) {
			td_mtx_lock_wrapper(&ct->mutex);
			ct->saving = true;
			td_mtx_unlock(&ct->mutex);
			//SaveImageJPG(ct->image_path.c_str(), ct->width, ct->height, ct->buffer);
			printf("%dx%d %s\n", ct->width, ct->height, ct->image_path.c_str());

			td_mtx_lock_wrapper(&ct->mutex);
			ct->saving = false;
			ct->save = false;
			td_mtx_unlock(&ct->mutex);
		}
		Sleep(1);
	}
	ct->running = false;*/
}

LRESULT CALLBACK WindowProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN && wParam == VK_F1)
		show_menu = !show_menu;
	if (show_menu) {
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse)
			return true; // Disable game input while menu is open
	}
	return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

BOOL wglSwapBuffersHook(HDC hDc) {
	static bool imGuiInitialized = false;
	if (!imGuiInitialized) {
		imGuiInitialized = true;
		HWND hGameWindow = WindowFromDC(hDc);
		hGameWindowProc = (WNDPROC)SetWindowLongPtr(hGameWindow, GWLP_WNDPROC, (LONG_PTR)WindowProcHook);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hGameWindow);
		ImGui_ImplOpenGL3_Init();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	}

	if (show_menu) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("DLL Utils", &show_menu, ImGuiWindowFlags_MenuBar);
		static bool show_vertex_editor = false;
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Plugins")) {
				ImGui::MenuItem("Vertex Editor", NULL, &show_vertex_editor);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		static float render_dist = 500;
		ImGui::Text("Render distance:");
		ImGui::SliderFloat("##render_dist", &render_dist, 100.0f, 1000.0f, "%.0f");
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
		if (ImGui::Button("Screenshot")) {
			//Game* game = (Game*)Teardown::GetGame();
			//Screenshot(game->screen_width, game->screen_height);
		}
		ImGui::End();

		if (show_vertex_editor) {
			Game* game = (Game*)Teardown::GetGame();
			if (game->editor != NULL && game->editor->selected != NULL) {
				ImGui::Begin("Vertex Editor", &show_vertex_editor);
				ImGui::Text("Entity type:");
				ImGui::SameLine();
				int type = game->editor->selected->type;
				bool valid = true;
				switch (type) {
				case Editor_Water:
					ImGui::Text("Water");
					break;
				case Editor_Voxagon:
					ImGui::Text("Voxagon");
					break;
				case Editor_Boundary:
					ImGui::Text("Boundary");
					break;
				case Editor_Trigger:
					ImGui::Text("Trigger");
					break;
				default:
					ImGui::Text("Invalid");
					valid = false;
					break;
				}
				if (valid) {
					ImGui::Text("Vertex count:");
					ImGui::SameLine();
					td_vector<Vertex>* vertices = &game->editor->selected->vertices;
					ImGui::Text("%d", vertices->getSize());

					ImGui::PushItemWidth(100);
					unsigned int i = 0;
					while (i < vertices->getSize()) {
						ImGui::PushID(i);
						ImGui::Text("%2d:", i + 1);
						ImGui::SameLine();
						ImGui::InputFloat("##x", &vertices->get(i).x, 0.1f, 10.0f, "%.1f");
						ImGui::SameLine();
						ImGui::InputFloat("##y", &vertices->get(i).y, 0.1f, 10.0f, "%.1f");
						ImGui::SameLine();
						if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
							if (i > 0) {
								Vertex temp = vertices->get(i);
								vertices->set(i, vertices->get(i - 1));
								vertices->set(i - 1, temp);
							}
						}
						ImGui::SameLine();
						if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
							if (i < vertices->getSize() - 1) {
								Vertex temp = vertices->get(i);
								vertices->set(i, vertices->get(i + 1));
								vertices->set(i + 1, temp);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("X##remove")) {
							vertices->remove(i);
							i--;
						}
						i++;
						ImGui::PopID();
					}
					ImGui::PopItemWidth();
				}
				ImGui::End();
			}
		}
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

	moduleBase = GetModuleHandleA("teardown.exe");
	td_lua_pushstring = (t_lua_pushstring)Teardown::GetReferenceTo(MEM_OFFSET::LuaPushString);
	td_lua_createtable = (t_lua_createtable)Teardown::GetReferenceTo(MEM_OFFSET::LuaCreateTable);
	td_RegisterLuaFunction = (t_RegisterLuaFunction)Teardown::GetReferenceTo(MEM_OFFSET::RegisterLuaFunction);
	td_mtx_lock_wrapper = (t_mutex)Teardown::GetReferenceTo(MEM_OFFSET::MutexLockWrapper);
	td_mtx_unlock = (t_mutex)Teardown::GetReferenceTo(MEM_OFFSET::MutexUnlock);
	t_RegisterGameFunctions rgf_addr = (t_RegisterGameFunctions)Teardown::GetReferenceTo(MEM_OFFSET::RegisterGameFunctions);
	//t_RegisterLuaFunction rlf_addr = (t_RegisterLuaFunction)Teardown::GetReferenceTo(MEM_OFFSET::RegisterLuaFunction);
	t_ScreenCaptureThread sct_addr = (t_ScreenCaptureThread)Teardown::GetReferenceTo(MEM_OFFSET::ScreenCaptureThread);

	Hook hook;
	hook.Create(L"opengl32.dll", "wglSwapBuffers", wglSwapBuffersHook, &td_wglSwapBuffers);
	hook.Create(rgf_addr, RegisterGameFunctionsHook, &td_RegisterGameFunctions);
	//hook.Create(rlf_addr, RegisterLuaFunctionHook, &td_RegisterLuaFunction);
	hook.Create(sct_addr, ScreenCaptureThreadHook, &td_ScreenCaptureThread);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
