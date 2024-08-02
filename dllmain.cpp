#include <vector>
#include <time.h>
#include <stdio.h>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "src/memory.h"
#include "src/recorder.h"
#include "src/extended_api.h"
#include "src/pros_override.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_dx12.h"

t_lua_createtable td_lua_createtable = nullptr;
t_lua_pushstring td_lua_pushstring = nullptr;

typedef BOOL WINAPI (*t_wglSwapBuffers) (HDC hDc);
t_wglSwapBuffers td_wglSwapBuffers = nullptr;

typedef void (*t_RegisterGameFunctions) (ScriptCore* core);
t_RegisterGameFunctions td_RegisterGameFunctions = nullptr;

typedef void (*t_ProcessVideoFrameOGL) (ScreenCapture* sc, int frame);
t_ProcessVideoFrameOGL td_ProcessVideoFrameOGL = nullptr;

typedef HRESULT (*t_CreateDXGIFactory1) (REFIID riid, void** ppFactory);
t_CreateDXGIFactory1 td_CreateDXGIFactory1 = nullptr;

typedef DWORD* (*t_InitRenderer) (DWORD* a1, int* a2);
t_InitRenderer td_InitRenderer = nullptr;

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

HMODULE moduleBase;
bool awwnb = false;
FastRecorder recorder;
ID3D12CommandQueue* d3d12CommandQueue = nullptr;

void RegisterGameFunctionsHook(ScriptCore* core) {
	td_RegisterGameFunctions(core);
	lua_State* L = core->inner_core.state_info->state;
	//const char* script_name = core->path.c_str();
	//printf("%p | Script: %s\n", (void*)L, script_name);

	RegisterLuaCFunctions(L);

	// TODO: improve, this may break if a script is spawned
	awwnb = false; // Reset remove boundary checkbox
	for (int i = 0; i < 16; i++)
		clock_init[i] = false;
}

void ProcessVideoFrameOGLHook(ScreenCapture* sc, int frame) {
	recorder.SetResolution(sc->width, sc->height);
	recorder.SetFolder(sc->capture_dir.c_str());
	recorder.AddFrame(sc->image_buffer);
}

// ----------------------------------------------------------------------------

bool show_menu = false;
WNDPROC hGameWindowProc;
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
	if (total_frames > 0)
		ImGui::ProgressBar((float)saved_frames / total_frames);
	if (ImGui::Button("Save recorded video"))
		recorder.SaveFrames();
	if (ImGui::Button("Clear"))
		recorder.ClearFrames();
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
	}
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
	Hook::Create(present_addr, PresentHook, &td_Present);
	return result;
}

HRESULT CreateDXGIFactory1Hook(REFIID riid, void** ppFactory) {
	HRESULT result = td_CreateDXGIFactory1(riid, ppFactory);
	void** factoryVTable = *reinterpret_cast<void***>(*ppFactory);
	t_CreateSwapChainForHwnd cscfh_addr = (t_CreateSwapChainForHwnd)factoryVTable[15];
	Hook::Create(cscfh_addr, CreateSwapChainForHwndHook, &td_CreateSwapChainForHwnd);
	return result;
}

DWORD* InitRendererHook(DWORD* param1, int* param2) {
	static const char* RENDERER_NAMES[] = {
		"OpenGL",
		"DirectX 12"
	};
	int index = *param2;
	printf("Current renderer is %s\n", RENDERER_NAMES[index]);
	return td_InitRenderer(param1, param2);
}

DWORD WINAPI MainThread(LPVOID lpThreadParameter) {
#ifdef _DEBUG_CONSOLE
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("TDLL Loaded\n");
#endif
	moduleBase = GetModuleHandleA("teardown.exe");
	td_lua_pushstring = (t_lua_pushstring)Teardown::GetReferenceTo(MEM_OFFSET::LuaPushString);
	td_lua_createtable = (t_lua_createtable)Teardown::GetReferenceTo(MEM_OFFSET::LuaCreateTable);

	Hook::Init();
	Hook::Create(L"opengl32.dll", "wglSwapBuffers", wglSwapBuffersHook, &td_wglSwapBuffers);
	Hook::Create(L"dxgi.dll", "CreateDXGIFactory1", CreateDXGIFactory1Hook, &td_CreateDXGIFactory1);
	t_RegisterGameFunctions rgf_addr = (t_RegisterGameFunctions)Teardown::GetReferenceTo(MEM_OFFSET::RegisterGameFunctions);
	Hook::Create(rgf_addr, RegisterGameFunctionsHook, &td_RegisterGameFunctions);
#ifdef _TDC
	t_ProcessVideoFrameOGL pvf_addr = (t_ProcessVideoFrameOGL)Teardown::GetReferenceTo(MEM_OFFSET::ProcessVideoFrameOGL);
	Hook::Create(pvf_addr, ProcessVideoFrameOGLHook, &td_ProcessVideoFrameOGL);
#endif
	t_InitRenderer ir_addr = (t_InitRenderer)Teardown::GetReferenceTo(MEM_OFFSET::InitRenderer);
	Hook::Create(ir_addr, InitRendererHook, &td_InitRenderer);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
