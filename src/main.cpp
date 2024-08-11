#include <d3d11.h>
#include <tchar.h>
#include "libaries/imgui/imgui.h"
#include "libaries/imgui/backends/imgui_impl_dx11.h"
#include "libaries/imgui/backends/imgui_impl_win32.h"
#include <windows.h>
#include "Renderer.h"
#include "UIManager.h"

// Global variables
HWND g_mainWindow = nullptr;
Renderer renderer;

// Forward declarations
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(int, char**) {
    // Window class setup
    WNDCLASSEXW wc = {
        sizeof(wc),
        CS_CLASSDC,
        WndProc,
        0L,
        0L,
        GetModuleHandle(nullptr),
        nullptr,
        nullptr,
        CreateSolidBrush(RGB(0, 0, 0)),
        nullptr,
        L"CameraViewerWindow",
        nullptr
    };
    ::RegisterClassExW(&wc);

    // Create window
    g_mainWindow = ::CreateWindowExW(0, wc.lpszClassName, L"CamVieX", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D renderer
    if (!renderer.Initialize(g_mainWindow)) {
        renderer.Cleanup();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(g_mainWindow, SW_SHOWDEFAULT);
    ::UpdateWindow(g_mainWindow);

    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Set up ImGui style and platform/renderer bindings
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(g_mainWindow);
    ImGui_ImplDX11_Init(renderer.g_pd3dDevice, renderer.g_pd3dDeviceContext);

    // Initialize UIManager with Direct3D device and context
    UIManager uiManager(renderer.g_pd3dDevice, renderer.g_pd3dDeviceContext);

    // Main loop
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }

        if (done)
            break;

        // Start the ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Render the UI
        uiManager.Render();

        // Rendering
        ImGui::Render();
        renderer.Render();
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    renderer.Cleanup();
    DestroyWindow(g_mainWindow);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (renderer.g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            renderer.Resize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
