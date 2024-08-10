#pragma once

#include <d3d11.h>
#include <windows.h>
#include <memory>

#include "libaries/imgui/imgui.h"
#include "libaries/imgui/backends/imgui_impl_dx11.h"
#include "libaries/imgui/backends/imgui_impl_win32.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(HWND hWnd);
    void Cleanup();
    void Render();
    void Resize(UINT width, UINT height);

    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
private:
    void CreateRenderTarget();
    void CleanupRenderTarget();

};
