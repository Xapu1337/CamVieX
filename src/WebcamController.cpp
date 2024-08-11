#include "WebcamController.h"
#include <iostream>

WebcamController::WebcamController(ID3D11Device* device, ID3D11DeviceContext* context)
    : is_initialized(false), new_frame_ready(false), callback(this), m_device(device), m_context(context), Width(640), Height(480) {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

WebcamController::~WebcamController() {
    Cleanup();
    CoUninitialize();
}

HRESULT WebcamController::Initialize(int cameraIndex) {
    if (is_initialized.load()) {
        return S_OK;
    }

    HRESULT hr;

    // Create Filter Graph Manager
    hr = pGraph.CoCreateInstance(CLSID_FilterGraph);
    if (FAILED(hr)) {
        return hr;
    }

    // Create Capture Graph Builder
    hr = pCaptureGraph.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pCaptureGraph->SetFiltergraph(pGraph);
    if (FAILED(hr)) {
        return hr;
    }

    // Enumerate Video Input Devices
    CComPtr<ICreateDevEnum> pDevEnum;
    hr = pDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED(hr)) {
        return hr;
    }

    CComPtr<IEnumMoniker> pEnum;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr != S_OK) {
        return E_FAIL;
    }

    IMoniker* pMoniker = nullptr;
    ULONG fetched;
    int index = 0;
    while (pEnum->Next(1, &pMoniker, &fetched) == S_OK) {
        if (index == cameraIndex) {
            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSourceFilter);
            pMoniker->Release();
            if (FAILED(hr)) {
                return hr;
            }
            break;
        }
        index++;
        pMoniker->Release();
    }

    if (!pSourceFilter) {
        return E_FAIL;
    }

    hr = pGraph->AddFilter(pSourceFilter, L"Video Capture");
    if (FAILED(hr)) {
        return hr;
    }

    // Add Sample Grabber Filter
    hr = AddFilterByCLSID(pGraph, CLSID_SampleGrabber, &pGrabberFilter, L"Sample Grabber");
    if (FAILED(hr)) {
        return hr;
    }

    hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
    if (FAILED(hr)) {
        return hr;
    }

    hr = ConfigureSampleGrabber();
    if (FAILED(hr)) {
        return hr;
    }

    // Add Null Renderer Filter
    hr = AddFilterByCLSID(pGraph, CLSID_NullRenderer, &pNullRenderer, L"Null Renderer");
    if (FAILED(hr)) {
        return hr;
    }

    // Connect Filters: Source Filter -> Sample Grabber -> Null Renderer
    hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSourceFilter, pGrabberFilter, pNullRenderer);
    if (FAILED(hr)) {
        return hr;
    }

    // Get Media Control Interface
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
    if (FAILED(hr)) {
        return hr;
    }

    // Create the Direct3D texture
    hr = CreateTexture(Width, Height);
    if (FAILED(hr)) {
        return hr;
    }

    is_initialized.store(true);
    return S_OK;
}

HRESULT WebcamController::AddFilterByCLSID(IGraphBuilder* pGraph, const GUID& clsid, IBaseFilter** ppF, const wchar_t* name) {
    CComPtr<IBaseFilter> pFilter;
    HRESULT hr = pFilter.CoCreateInstance(clsid);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pGraph->AddFilter(pFilter, name);
    if (FAILED(hr)) {
        return hr;
    }

    *ppF = pFilter.Detach();
    return S_OK;
}

HRESULT WebcamController::ConfigureSampleGrabber() {
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;

    HRESULT hr = pGrabber->SetMediaType(&mt);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pGrabber->SetOneShot(FALSE);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pGrabber->SetBufferSamples(TRUE);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pGrabber->SetCallback(&callback, 1);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

HRESULT WebcamController::CreateTexture(int width, int height) {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // Create the front buffer texture
    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &m_texture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create front buffer texture. HRESULT: " << std::hex << hr << std::endl;
        return hr;
    }

    // Create the back buffer texture
    hr = m_device->CreateTexture2D(&desc, nullptr, &m_backBufferTexture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create back buffer texture. HRESULT: " << std::hex << hr << std::endl;
        return hr;
    }

    // Create the shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv);
    if (FAILED(hr)) {
        std::cerr << "Failed to create shader resource view. HRESULT: " << std::hex << hr << std::endl;
        return hr;
    }

    Width = width;
    Height = height;

    return S_OK;
}


HRESULT WebcamController::StartCapture() {
    if (!is_initialized.load()) {
        return E_FAIL;
    }

    if (!pControl) {
        return E_POINTER;
    }

    HRESULT hr = pControl->Run();
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

HRESULT WebcamController::StopCapture() {
    if (!is_initialized.load()) {
        return E_FAIL;
    }

    if (!pControl) {
        return E_POINTER;
    }

    HRESULT hr = pControl->Stop();
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

ID3D11ShaderResourceView* WebcamController::GetFrameTexture() {
    if (new_frame_ready.load()) {
        new_frame_ready.store(false);
        return m_srv.Get();
    }
    return nullptr;
}

std::vector<std::wstring> WebcamController::ListAvailableCameras() {
    std::vector<std::wstring> cameraNames;

    CComPtr<ICreateDevEnum> pDevEnum;
    HRESULT hr = pDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED(hr)) {
        return cameraNames;
    }

    CComPtr<IEnumMoniker> pEnum;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr != S_OK) {
        return cameraNames;
    }

    IMoniker* pMoniker = nullptr;
    ULONG fetched;
    while (pEnum->Next(1, &pMoniker, &fetched) == S_OK) {
        CComPtr<IPropertyBag> pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
        if (SUCCEEDED(hr)) {
            VARIANT varName;
            VariantInit(&varName);
            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            if (SUCCEEDED(hr)) {
                cameraNames.push_back(varName.bstrVal);
            }
            VariantClear(&varName);
        }
        pMoniker->Release();
    }

    return cameraNames;
}

void WebcamController::Cleanup() {
    if (is_initialized.load()) {
        if (pControl) {
            pControl->Stop();
        }

        if (pGraph && pGrabberFilter && pNullRenderer) {
            pGraph->RemoveFilter(pNullRenderer);
            pGraph->RemoveFilter(pGrabberFilter);
        }

        if (pGrabberFilter) {
            pGrabberFilter.Release();
            pGrabber.Detach();
            pGrabber.Release();
        }

        if (pSourceFilter) {
            pGraph->RemoveFilter(pSourceFilter);
            pSourceFilter.Release();
        }

        pNullRenderer.Release();
        pControl.Release();
        pGraph.Release();
        pCaptureGraph.Release();
        pCamCtrl.Release();
        pProcAmp.Release();

        pGrabberFilter = nullptr;
        pGrabber = nullptr;
        pSourceFilter = nullptr;
        pNullRenderer = nullptr;
        pControl = nullptr;
        pGraph = nullptr;
        pCaptureGraph = nullptr;
        pCamCtrl = nullptr;
        pProcAmp = nullptr;

        is_initialized.store(false);
    }
}
