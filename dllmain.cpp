#include <time.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <atomic>
#include <thread>

#include "extended_api.h"
#include "pros_override.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
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

typedef void (*t_ProcessVideoFrameOGL) (ScreenCapture* sc, int frame);
t_ProcessVideoFrameOGL td_ProcessVideoFrameOGL = nullptr;

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

void RegisterGameFunctionsHook(ScriptCore* core) {
	td_RegisterGameFunctions(core);
	lua_State* L = core->inner_core.state_info->state;
	//const char* script_name = core->path.c_str();
	//printf("%p | Script: %s\n", (void*)L, script_name);

	RegisterLuaCFunctions(L);
	if (core->check_internal != NULL && core->check_internal->privilege > 1)
		core->check_internal->privilege = 1;

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

void SaveImageJPG(const char* path, int width, int height, uint8_t* pixels) {
	FlipImageVertically(width, height, pixels);
	stbi_write_jpg(path, width, height, 3, pixels, 80);
}

class FastRecorder {
	#define THREAD_COUNT 5					// One thread is always idle
	#define SUGGESTED_BUFFER_SIZE 60		// Number of frames to save per thread, can be bigger
	int width;
	int height;
	int total_frames;						// The total number of frames
	int current_buffer;						// The current buffer to write frames to
	std::string capture_dir;
	std::atomic<int> saved_frames;			// The number of frames saved to disk
	std::atomic<int> sequence_number;		// The current file for raw data
	int starting_frame[THREAD_COUNT];		// The starting frame index for each buffer
	std::thread save_thread[THREAD_COUNT];
	std::atomic<bool> running[THREAD_COUNT];// The state of the save thread
	std::vector<uint8_t*> video_frames[THREAD_COUNT];

	void SaveThread(int index) {
		unsigned int start = starting_frame[index];
		unsigned int count = video_frames[index].size();
		//printf("Thread %d (%2d frames) [%4d -> %4d]\n", index, count, start, start + count - 1);
		for (unsigned int i = 0; i < count; i++) {
			char image_path[256];
			snprintf(image_path, sizeof(image_path), "%s/%04d.jpg", capture_dir.c_str(), start + i);
			if (video_frames[index][i] != NULL) {
				SaveImageJPG(image_path, width, height, video_frames[index][i]);
				delete[] video_frames[index][i];
				video_frames[index][i] = NULL;
			}
			saved_frames++;
		}
		video_frames[index].clear(); // Clear vector for next batch
		//printf("Thread %d finished!\n", index);
		Sleep(1);
		running[index] = false; // Signal that the thread has finished
	}

	void DumpRawData(int index) {
		char file_path[256];
		snprintf(file_path, sizeof(file_path), "%s/%04d.tdc", capture_dir.c_str(), sequence_number++);
		FILE* file = fopen(file_path, "wb");
		if (file != NULL) {
			int frame_count = video_frames[index].size();
			fwrite(&starting_frame[index], sizeof(int), 1, file);
			fwrite(&frame_count, sizeof(int), 1, file);
			fwrite(&width, sizeof(int), 1, file);
			fwrite(&height, sizeof(int), 1, file);
			for (unsigned int i = 0; i < video_frames[index].size(); i++) {
				if (video_frames[index][i] != NULL) {
					fwrite(video_frames[index][i], 3 * width * height, 1, file);
					delete[] video_frames[index][i];
					video_frames[index][i] = NULL;
				}
				saved_frames++;
			}
			video_frames[index].clear();
			fclose(file);
		}
		running[index] = false;
	}
public:
	FastRecorder() {
		width = 0;
		height = 0;
		saved_frames = 0;
		current_buffer = 0;
		sequence_number = 0;
		capture_dir = "mods/screenrecorder";
		for (int i = 0; i < THREAD_COUNT; i++) {
			starting_frame[i] = 0;
			video_frames[i].reserve(SUGGESTED_BUFFER_SIZE);
			running[i] = false;
		}
	}

	void SetResolution(int width, int height) {
		this->width = width;
		this->height = height;
	}

	void SetFolder(const char* dir) {
		capture_dir = dir;
	}

	void AddFrame(uint8_t* pixels) {
		int size = 3 * width * height;
		uint8_t* frame = new uint8_t[size];
		memcpy(frame, pixels, size);
		video_frames[current_buffer].push_back(frame);
		total_frames++;
		if (total_frames % SUGGESTED_BUFFER_SIZE == 0)
			SaveFrames();
	}

	int GetSavedFrames() {
		return saved_frames.load();
	}

	int GetTotalFrames() {
		return total_frames;
	}

	void SaveFrames() {
		bool found = false;
		int next_buffer = current_buffer;
		do {
			next_buffer = (next_buffer + 1) % THREAD_COUNT;
			if (next_buffer != current_buffer && !running[next_buffer]) {
				if (save_thread[next_buffer].joinable())
					save_thread[next_buffer].join();

				// Start current thread if the next buffer is available for use
				running[current_buffer] = true;
				save_thread[current_buffer] = std::thread(&FastRecorder::SaveThread, this, current_buffer);

				found = true;
				current_buffer = next_buffer;
				starting_frame[current_buffer] = total_frames - 1; // Set starting frame for next buffer
			}
		} while (!found && next_buffer != current_buffer);
	}

	void ClearFrames() {
		total_frames = 0;
		saved_frames = 0;
		current_buffer = 0;
		sequence_number = 0;
		for (int i = 0; i < THREAD_COUNT; i++) {
			for (unsigned int j = 0; j < video_frames[i].size(); j++) {
				if (video_frames[i][j] != NULL) {
					delete[] video_frames[i][j];
					video_frames[i][j] = NULL;
				}
			}
			video_frames[i].clear();
			starting_frame[i] = 0;
			running[i] = false;
			if (save_thread[i].joinable())
				save_thread[i].join();
		}
	}

	~FastRecorder() {
		ClearFrames();
	}
};

FastRecorder recorder;

void ProcessVideoFrameOGLHook(ScreenCapture* sc, int frame) {
	recorder.SetResolution(sc->width, sc->height);
	recorder.SetFolder(sc->capture_dir.c_str());
	recorder.AddFrame(sc->image_buffer);
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

		ImGui::Text("Saved video frames:");
		ImGui::SameLine();
		int saved_frames = recorder.GetSavedFrames();
		int total_frames = recorder.GetTotalFrames();
		ImGui::Text("%d / %d", saved_frames, total_frames);
		ImGui::ProgressBar((float)saved_frames / total_frames);
		if (ImGui::Button("Save recorded video")) {
			recorder.SaveFrames();
		}
		if (ImGui::Button("Clear")) {
			recorder.ClearFrames();
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
					case EditorEntityTypes::Water:
						ImGui::Text("Water");
						break;
					case EditorEntityTypes::Voxagon:
						ImGui::Text("Voxagon");
						break;
					case EditorEntityTypes::Boundary:
						ImGui::Text("Boundary");
						break;
					case EditorEntityTypes::Trigger:
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
					td_vector<Vec2>* vertices = &game->editor->selected->vertices;
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
								Vec2 temp = vertices->get(i);
								vertices->set(i, vertices->get(i - 1));
								vertices->set(i - 1, temp);
							}
						}
						ImGui::SameLine();
						if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
							if (i < vertices->getSize() - 1) {
								Vec2 temp = vertices->get(i);
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
	Sleep(1000);
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
	t_RegisterGameFunctions rgf_addr = (t_RegisterGameFunctions)Teardown::GetReferenceTo(MEM_OFFSET::RegisterGameFunctions);
	t_ProcessVideoFrameOGL pvf_addr = (t_ProcessVideoFrameOGL)Teardown::GetReferenceTo(MEM_OFFSET::ProcessVideoFrameOGL);

	Hook hook;
	hook.Create(L"opengl32.dll", "wglSwapBuffers", wglSwapBuffersHook, &td_wglSwapBuffers);
	hook.Create(rgf_addr, RegisterGameFunctionsHook, &td_RegisterGameFunctions);
#ifdef TDC
	hook.Create(pvf_addr, ProcessVideoFrameOGLHook, &td_ProcessVideoFrameOGL);
#endif
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
