#include <assert.h>
#include <mutex>
#include <stdio.h>
#include <thread>
#include <time.h>
#include <vector>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "src/memory.h"
#include "src/recorder.h"
#include "src/extended_api.h"
#include "src/winmm_proxy.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_dx12.h"

typedef BOOL WINAPI (*t_wglSwapBuffers) (HDC hDc);
t_wglSwapBuffers td_wglSwapBuffers = nullptr;

typedef HRESULT (*t_CreateDXGIFactory1) (REFIID riid, void** ppFactory);
t_CreateDXGIFactory1 td_CreateDXGIFactory1 = nullptr;

typedef HRESULT (*t_CreateSwapChainForHwnd) (
	IDXGIFactory* pFactory,
	IUnknown* pDevice,
	HWND hWnd,
	const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc,
	IDXGIOutput* pRestrictToOutput,
	IDXGISwapChain1** ppSwapChain
);
t_CreateSwapChainForHwnd td_CreateSwapChainForHwnd = nullptr;

typedef HRESULT (*t_Present) (IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags);
t_Present td_Present = nullptr;

bool awwnb = false;
FastRecorder recorder;

Broadcast broadcast;
std::mutex msg_mutex;
std::vector<std::string> messages;

void InitScriptInnerLoopHook(ScriptCoreInner* inner_core) {
	td_InitScriptInnerLoop(inner_core);
	// TODO: move to start level
	// TODO: reset clock
	awwnb = false; // Reset remove boundary checkbox

	lua_State* L = inner_core->state_info->state;
	//const char* script_name = core->path.c_str();
	//printf("%p | Script: %s\n", (void*)L, script_name);
	RegisterLuaCFunctions(L);
}

void ProcessVideoFrameGLHook(ScreenCapture* sc, int frame) {
	recorder.SetResolution(sc->width, sc->height);
	recorder.SetFolder(sc->capture_dir.c_str());
	recorder.AddFrame(sc->image_buffer);
}

// ----------------------------------------------------------------------------

HMODULE moduleBase;
bool show_menu = false;
WNDPROC hGameWindowProc;
ID3D12CommandQueue* d3d12CommandQueue = nullptr;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN && wParam == VK_F1)
		show_menu = !show_menu;
	if (show_menu) {
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse)
			return true;
	}
	return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

// ----------------------------------------------------------------------------

void ImGuiRenderFrame() {
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

	/*static float far_plane = 500.0f;
	ImGui::Text("Max render distance:");
	ImGui::SliderFloat("##far_plane", &far_plane, 100.0f, 1000.0f, "%.0f");
	ImGui::InputFloat("##far_plane_input", &far_plane, 1.0f, 10.0f, "%.0f");
	if (ImGui::Button("Set max render distance")) {
		float* far_plane_addr = (float*)Teardown::GetReferenceTo(MEM_OFFSET::FarPlane);
		Patch(far_plane_addr, &far_plane);
	}

	static float near_plane = 0.05f;
	ImGui::Text("Min render distance:");
	ImGui::SliderFloat("##near_plane", &near_plane, 0.01f, 100.0f, "%.2f");
	ImGui::InputFloat("##near_plane_input", &near_plane, 0.01f, 1.0f, "%.2f");
	if (ImGui::Button("Set min render distance")) {
		float* near_plane_addr = (float*)Teardown::GetReferenceTo(0x92FE58);
		Patch(near_plane_addr, &near_plane);
	}*/

	if (ImGui::Checkbox("Remove boundaries", &awwnb) && awwnb) {
		Game* game = (Game*)Teardown::GetGame();
		game->scene->boundary.clear();
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
	if (total_frames > 0)
		ImGui::ProgressBar((float)saved_frames / total_frames);

	bool disabled = recorder.GetTotalFrames() == 0;
	ImGui::BeginDisabled(disabled);
	if (ImGui::Button("Save recorded video"))
		recorder.SaveFrames();
	ImGui::EndDisabled();

	if (ImGui::Button("Clear"))
		recorder.ClearFrames();
	ImGui::End();

	/*if (show_vertex_editor) {
		Game* game = (Game*)Teardown::GetGame();
		if (game->editor != NULL && game->editor->selected != NULL) {
			ImGui::Begin("Vertex Editor", &show_vertex_editor);
			ImGui::Text("Entity type:");
			ImGui::SameLine();
			int type = game->editor->selected->type;
			bool valid = true;
			switch (type) {
				case EditorEntityType::Water:
					ImGui::Text("Water");
					break;
				case EditorEntityType::Voxagon:
					ImGui::Text("Voxagon");
					break;
				case EditorEntityType::Boundary:
					ImGui::Text("Boundary");
					break;
				case EditorEntityType::Trigger:
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
	}*/
	ImGui::Render();
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

		ImGui::StyleColorsDark();
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	}

	if (show_menu) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGuiRenderFrame();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	return td_wglSwapBuffers(hDc);
}

struct FrameContext {
	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12Resource* main_render_target_resource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE main_render_target_descriptor;
};

HRESULT PresentHook(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags) {
	static UINT buffersCounts = 0;
	static HWND hGameWindow = nullptr;
	static ID3D12Device* d3d12Device = nullptr;
	static FrameContext* frameContext = nullptr;
	static ID3D12GraphicsCommandList* d3d12CommandList = nullptr;
	static ID3D12DescriptorHeap* d3d12DescriptorHeapImGuiRender = nullptr;
	static ID3D12DescriptorHeap* d3d12DescriptorHeapBackBuffers = nullptr;

	static bool imGuiInitialized = false;
	if (!imGuiInitialized) {
		imGuiInitialized = true;

		CreateEvent(nullptr, FALSE, FALSE, nullptr);

		DXGI_SWAP_CHAIN_DESC sdesc;
		swapChain->GetDevice(__uuidof(ID3D12Device), (void**)&d3d12Device);
		swapChain->GetDesc(&sdesc);
		swapChain->GetHwnd(&hGameWindow);
		sdesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sdesc.OutputWindow = hGameWindow;
		sdesc.Windowed = (GetWindowLongPtr(hGameWindow, GWL_STYLE) & WS_POPUP) == 0;

		buffersCounts = sdesc.BufferCount;
		frameContext = new FrameContext[buffersCounts];

		D3D12_DESCRIPTOR_HEAP_DESC descriptorImGuiRender = {};
		descriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorImGuiRender.NumDescriptors = buffersCounts;
		descriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		d3d12Device->CreateDescriptorHeap(&descriptorImGuiRender, IID_PPV_ARGS(&d3d12DescriptorHeapImGuiRender));

		ID3D12CommandAllocator* allocator;
		d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
		for (size_t i = 0; i < buffersCounts; i++)
			frameContext[i].commandAllocator = allocator;

		d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, NULL, IID_PPV_ARGS(&d3d12CommandList));

		D3D12_DESCRIPTOR_HEAP_DESC descriptorBackBuffers;
		descriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorBackBuffers.NumDescriptors = buffersCounts;
		descriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorBackBuffers.NodeMask = 1;

		d3d12Device->CreateDescriptorHeap(&descriptorBackBuffers, IID_PPV_ARGS(&d3d12DescriptorHeapBackBuffers));

		const auto rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

		for (UINT i = 0; i < buffersCounts; i++) {
			ID3D12Resource* pBackBuffer = nullptr;
			frameContext[i].main_render_target_descriptor = rtvHandle;
			swapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
			d3d12Device->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
			frameContext[i].main_render_target_resource = pBackBuffer;
			rtvHandle.ptr += rtvDescriptorSize;
		}

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hGameWindow);
		ImGui_ImplDX12_Init(d3d12Device, buffersCounts,
			DXGI_FORMAT_R8G8B8A8_UNORM, d3d12DescriptorHeapImGuiRender,
			d3d12DescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(),
			d3d12DescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart()
		);

		ImGui_ImplDX12_CreateDeviceObjects();
		hGameWindowProc = (WNDPROC)SetWindowLongPtrW(hGameWindow, GWLP_WNDPROC, (LONG_PTR)WindowProcHook);

		ImGui::StyleColorsClassic();
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	}

	if (show_menu) {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();

		FrameContext& currentFrameContext = frameContext[swapChain->GetCurrentBackBufferIndex()];
		currentFrameContext.commandAllocator->Reset();

		D3D12_RESOURCE_BARRIER barrier; // Bari Bari no Mi
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = currentFrameContext.main_render_target_resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		d3d12CommandList->Reset(currentFrameContext.commandAllocator, nullptr);
		d3d12CommandList->ResourceBarrier(1, &barrier);
		d3d12CommandList->OMSetRenderTargets(1, &currentFrameContext.main_render_target_descriptor, FALSE, nullptr);
		d3d12CommandList->SetDescriptorHeaps(1, &d3d12DescriptorHeapImGuiRender);

		ImGuiRenderFrame();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), d3d12CommandList);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		d3d12CommandList->ResourceBarrier(1, &barrier);
		d3d12CommandList->Close();

		d3d12CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&d3d12CommandList));
	}
	return td_Present(swapChain, syncInterval, flags);
}

HRESULT CreateSwapChainForHwndHook(
	IDXGIFactory* pFactory,
	IUnknown* pDevice,
	HWND hWnd,
	const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc,
	IDXGIOutput* pRestrictToOutput,
	IDXGISwapChain1** ppSwapChain
) {
	d3d12CommandQueue = (ID3D12CommandQueue*)pDevice;
	HRESULT result = td_CreateSwapChainForHwnd(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
	void** vTable = *reinterpret_cast<void***>(*ppSwapChain);
	t_Present present_addr = (t_Present)vTable[8];
	Hook::Create(present_addr, PresentHook, &td_Present, "Present");
	return result;
}

HRESULT CreateDXGIFactory1Hook(REFIID riid, void** ppFactory) {
	HRESULT result = td_CreateDXGIFactory1(riid, ppFactory);
	void** factoryVTable = *reinterpret_cast<void***>(*ppFactory);
	t_CreateSwapChainForHwnd cscfh_addr = (t_CreateSwapChainForHwnd)factoryVTable[15];
	Hook::Create(cscfh_addr, CreateSwapChainForHwndHook, &td_CreateSwapChainForHwnd, "CreateSwapChainForHwnd");
	return result;
}

void ReceiveThread() {
	char buffer[1024];
	while (true) {
		memset(buffer, 0, 1024);
		broadcast.Receive(buffer, 1024);
		msg_mutex.lock();
		messages.push_back(buffer);
		msg_mutex.unlock();
	}
}

DWORD WINAPI MainThread(LPVOID lpThreadParameter) {
#ifdef _DEBUG_CONSOLE
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("TDLL Loaded\n");
#endif

	Hook::Init();
	Hook::Create(L"opengl32.dll", "wglSwapBuffers", wglSwapBuffersHook, &td_wglSwapBuffers);
	Hook::Create(L"dxgi.dll", "CreateDXGIFactory1", CreateDXGIFactory1Hook, &td_CreateDXGIFactory1);
	Sleep(1000);

	moduleBase = GetModuleHandleA("teardown.exe");
	td_lua_pushstring = (t_lua_pushstring)FindPatternInModule(moduleBase, luaPushStringPattern);
	td_lua_createtable = (t_lua_createtable)FindPatternInModule(moduleBase, luaCreateTablePattern);
	t_InitScriptInnerLoop isil_addr = (t_InitScriptInnerLoop)FindPatternInModule(moduleBase, initScriptInnerLoopPattern);
	uintptr_t get_flashlight_addr = FindPatternInModule(moduleBase, getFlashlightPattern);
	if (!td_lua_pushstring || !td_lua_createtable || !get_flashlight_addr || !isil_addr) {
		MessageBoxA(nullptr, "TDLL is not compatible with this version of Teardown.\nPlease update or remove.", "TDLL - Invalid version", MB_ICONERROR | MB_OK);
		return 0;
	}

	// MOV RAX, [game]
	const int MOV_HEADER_SIZE = 3;
	const int MOV_INSTRUCTION_SIZE = 7;
	uint32_t game_addr_offset = 0;
	memcpy(&game_addr_offset, (void*)(get_flashlight_addr + MOV_HEADER_SIZE), sizeof(uint32_t));
	//uintptr_t get_flashlight_offset = get_flashlight_addr - (uintptr_t)moduleBase;
	//MEM_OFFSET::Game = get_flashlight_offset + game_addr_offset + MOV_INSTRUCTION_SIZE;
	game_ptr = *(Game**)(get_flashlight_addr + game_addr_offset + MOV_INSTRUCTION_SIZE);

	Hook::Create(isil_addr, InitScriptInnerLoopHook, &td_InitScriptInnerLoop, "InitScriptInnerLoop");
#ifdef _TDC
	t_ProcessVideoFrameGL pvf_addr = (t_ProcessVideoFrameGL)FindPatternInModule(moduleBase, processVideoFrameGLPattern);
	Hook::Create(pvf_addr, ProcessVideoFrameGLHook, &td_ProcessVideoFrameGL);
#endif

	try {
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
		broadcast.Init(3802);
		std::thread receiveThread(ReceiveThread);
		receiveThread.detach();
	} catch (std::exception& e) {
		printf("[ERROR] %s\n", e.what());
	}

	// teardown.exe+64E55 - F3 44 0F10 05 82F88500  - movss xmm8,[teardown.exe+8C46E0] { (0.05) }
	/*uintptr_t load_near_plane_addr = Teardown::GetReferenceTo(0x64E55);
	uintptr_t new_near_plane_addr = Teardown::GetReferenceTo(0x92FE58); // Some ~hopefully~ unused memory, initialized to the default value
	int32_t relative_offset = (int32_t)(new_near_plane_addr - (load_near_plane_addr + 9));
	uint8_t load_near_plane_bytes[] = { 0xF3, 0x44, 0x0F, 0x10, 0x05, 0x82, 0xF8, 0x85, 0x00 };
	memcpy(&load_near_plane_bytes[5], &relative_offset, sizeof(relative_offset));
	PatchBA((BYTE*)load_near_plane_addr, (BYTE*)load_near_plane_bytes, sizeof(load_near_plane_bytes));*/
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
