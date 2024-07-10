#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "util.h"

struct Size
{
    int x;
    int y;
};
struct Size size = {800, 600};

int counter = 0;
// Declare g_Pos as a global variable
POINTS g_Pos = {};
bool g_IsDragging = false;

// Define the dragging area
const int DRAGGING_AREA_WIDTH = 750;
const int DRAGGING_AREA_HEIGHT = 30;

// Data
static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
static std::thread g_renderThread;
static std::mutex g_resizeMutex;
static std::condition_variable g_resizeCondition;
static bool g_resizeRequested = false;
static bool g_done = false;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void RenderLoop()
{
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    ImVec4 clear_color = HexToIv4(0x313244);
    ImVec4 custom_bg_color = HexToIv4(0x585b70);     // Custom background color
    ImVec4 custom_slider_color = HexToIv4(0xa6e3a1); // Custom slider color
    ImVec4 red_color = HexToIv4(0xf38ba8);

    while (!g_done)
    {
        // Handle window resize
        {
            std::unique_lock<std::mutex> lock(g_resizeMutex);
            if (g_resizeRequested)
            {
               /* CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
                g_ResizeWidth = g_ResizeHeight = 0;
                CreateRenderTarget();
                g_resizeRequested = false; */
            }
        }

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            //::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        // Your ImGui rendering code here
        static float f = 0.0f;
        /*
        ImGui::SetNextWindowPos(ImVec2(100, 100));
        ImGui::SetNextWindowSize(ImVec2(400, 300));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f); // Set window rounding to 10 pixels
        ImGui::PushStyleColor(ImGuiCol_WindowBg, custom_bg_color);
        ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::Text("This is some useful text.");

        ImGui::PushStyleColor(ImGuiCol_SliderGrab, custom_slider_color);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, custom_slider_color);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(); // Pop the window rounding style

        ImGui::PushStyleColor(ImGuiCol_Button, custom_slider_color);
        if (ImGui::Button("Button"))
            counter++;
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
        ImGui::PopStyleColor();

        ImGui::SetNextWindowPos(ImVec2(400, 400));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, custom_slider_color);
        ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::Text("This is some useful text.");
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::End();
        ImGui::PopStyleColor();
        */
        ImGui::PushStyleColor(ImGuiCol_WindowBg, clear_color);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::SetNextWindowPos(ImVec2(735, -10));
        ImVec2 buttonSize = ImVec2(50.0f, 30.0f);

        ImGui::Begin("Window 1", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        // Set button coordinates (relative to the window)
        ImVec2 buttonPosition = ImVec2(0.0f, 0.0f);
        ImGui::SetCursorPos(buttonPosition);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,red_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,HexToIv4(0xf77499));
        if (ImGui::Button("X", buttonSize))
            counter++;
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar(); // Pop the window rounding style
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_WindowBg, custom_bg_color);
        ImGui::Begin("Window 2", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
        ImGui::PopStyleColor();



        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0); // Present with vsync
        // HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
    // Create application window
    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr};
    ::RegisterClassExW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2.0f) - (size.x / 2.f), (GetSystemMetrics(SM_CYSCREEN) / 2.0f) - (size.y / 2.f), size.x, size.y, nullptr, nullptr, wc.hInstance, nullptr);
    //::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = {-1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Start the render thread
    g_renderThread = std::thread(RenderLoop);

    // Main loop
    MSG msg;
    while (!g_done)
    {
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                g_done = true;
        }
        if (g_done)
            break;
        if (counter != 0)
        {
            g_done = true;
        }
    }

    // Cleanup
    g_renderThread.join();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        g_Pos = MAKEPOINTS(lParam);

        // Check if the click is within the draggable area
        if (g_Pos.x >= 0 && g_Pos.x <= DRAGGING_AREA_WIDTH && g_Pos.y >= 0 && g_Pos.y <= DRAGGING_AREA_HEIGHT)
        {
            g_IsDragging = true;
        }
        else
        {
            g_IsDragging = false;
        }
        return 0L;
    }

    case WM_LBUTTONUP:
    {
        g_IsDragging = false;
        return 0L;
    }

    case WM_MOUSEMOVE:
    {
        if (g_IsDragging)
        {
            const auto points = MAKEPOINTS(lParam);
            auto rect = RECT{};

            GetWindowRect(hWnd, &rect);

            rect.left += points.x - g_Pos.x;
            rect.top += points.y - g_Pos.y;

            SetWindowPos(
                hWnd,
                HWND_TOPMOST,
                rect.left, rect.top,
                0, 0,
                SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
        }

        return 0L;
    }
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain)
    {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext)
    {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

void CreateRenderTarget()
{
    ID3D11Texture2D *pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}
