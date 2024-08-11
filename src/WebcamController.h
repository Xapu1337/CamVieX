#pragma once

#include <windows.h>
#include <dshow.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include "graph.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "d3d11.lib")

class WebcamController {
public:
    WebcamController(ID3D11Device* device, ID3D11DeviceContext* context);
    ~WebcamController();

    HRESULT Initialize(int cameraIndex = 0);
    HRESULT StartCapture();
    HRESULT StopCapture();
    ID3D11ShaderResourceView* GetFrameTexture();
    std::vector<std::wstring> ListAvailableCameras();

private:
    // COM interfaces
    CComPtr<IGraphBuilder> pGraph;
    CComPtr<ICaptureGraphBuilder2> pCaptureGraph;
    CComPtr<IMediaControl> pControl;
    CComPtr<IBaseFilter> pSourceFilter;
    CComPtr<IBaseFilter> pGrabberFilter;
    CComPtr<ISampleGrabber> pGrabber;
    CComPtr<IBaseFilter> pNullRenderer;
    CComPtr<IAMCameraControl> pCamCtrl;
    CComPtr<IAMVideoProcAmp> pProcAmp;

    std::atomic<bool> is_initialized;
    std::atomic<bool> new_frame_ready;

    // Direct3D resources
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backBufferTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

    // Frame buffer
    std::mutex frame_mutex;

    // Helper methods
    HRESULT AddFilterByCLSID(IGraphBuilder* pGraph, const GUID& clsid, IBaseFilter** ppF, const wchar_t* name);
    HRESULT ConfigureSampleGrabber();
    void Cleanup();
    HRESULT CreateTexture(int width, int height);

    // SampleGrabber Callback
    class SampleGrabberCallback : public ISampleGrabberCB {
    public:
        SampleGrabberCallback(WebcamController* controller) : controller(controller) {}

        STDMETHODIMP SampleCB(double Time, IMediaSample* pSample) override {
            return E_NOTIMPL;
        }

        STDMETHODIMP BufferCB(double Time, BYTE* pBuffer, long BufferLen) override {
            std::lock_guard<std::mutex> lock(controller->frame_mutex);

            if (controller->new_frame_ready.load()) {
                return S_OK; // Avoid overwriting if the frame hasn't been processed yet
            }

            // Ensure back buffer texture is valid
            if (!controller->m_backBufferTexture) {
                return E_FAIL;  // No back buffer texture available
            }

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = controller->m_context->Map(controller->m_backBufferTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (FAILED(hr)) {
                return hr;
            }

            // Copy the buffer data to the back buffer texture
            BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
            for (int y = 0; y < controller->Height; ++y) {
                memcpy(mappedData + y * mappedResource.RowPitch, pBuffer + y * controller->Width * 3, controller->Width * 3);
            }

            controller->m_context->Unmap(controller->m_backBufferTexture.Get(), 0);

            // Swap front and back buffers
            std::swap(controller->m_texture, controller->m_backBufferTexture);

            controller->new_frame_ready.store(true);

            return S_OK;
        }

        STDMETHODIMP_(ULONG) AddRef() override {
            return 1;
        }

        STDMETHODIMP_(ULONG) Release() override {
            return 2;
        }

        STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
            if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
                *ppv = static_cast<void*>(this);
                return S_OK;
            }
            return E_NOINTERFACE;
        }

    private:
        WebcamController* controller;
    };

    SampleGrabberCallback callback;
    int Width, Height;
};

