#include "CameraManager.h"
#include <iostream>

CameraManager::CameraManager() {
    CoInitialize(nullptr);
}

CameraManager::~CameraManager() {
    close();
    CoUninitialize();
}

bool CameraManager::open(int cameraIndex) {
    if (opened) {
        std::cerr << "Camera already opened.\n";
        return false;
    }

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pGraph));
    if (FAILED(hr)) {
        std::cerr << "Failed to create Filter Graph. HRESULT: " << hr << "\n";
        return false;
    }

    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pBuilder));
    if (FAILED(hr)) {
        std::cerr << "Failed to create Capture Graph Builder. HRESULT: " << hr << "\n";
        return false;
    }

    hr = pGraph->QueryInterface(IID_PPV_ARGS(&pControl));
    if (FAILED(hr)) {
        std::cerr << "Failed to create Media Control. HRESULT: " << hr << "\n";
        return false;
    }

    this->cameraIndex = cameraIndex;
    initializeGraph();

    opened = true;
    return true;
}

void CameraManager::close() {
    // CComPtr automatically handles resource cleanup
    pCamControl.Release();
    pProcAmp.Release();
    pVideoCapture.Release();
    pControl.Release();
    pBuilder.Release();
    pGraph.Release();

    opened = false;
}

bool CameraManager::captureFrame() {
    if (!opened) {
        std::cerr << "Camera not opened.\n";
        return false;
    }

    // Implement frame capture logic

    return true;
}

void CameraManager::setProperty(long property, long value) {
    if (!pCamControl) {
        std::cerr << "Camera control not available.\n";
        return;
    }
    HRESULT hr = pCamControl->Set(property, value, CameraControl_Flags_Manual);
    if (FAILED(hr)) {
        std::cerr << "Failed to set property " << property << ". HRESULT: " << hr << "\n";
    }
}

long CameraManager::getProperty(long property) {
    long value = 0, flags = 0;
    if (pCamControl) {
        HRESULT hr = pCamControl->Get(property, &value, &flags);
        if (FAILED(hr)) {
            std::cerr << "Failed to get property " << property << ". HRESULT: " << hr << "\n";
        }
    }
    return value;
}

void CameraManager::dumpCameraInfo() {
    if (!pCamControl) {
        std::cerr << "Camera control not available.\n";
        return;
    }

    std::cout << "Camera Info:\n";
    long minValue, maxValue, step, defaultValue, flags;
    for (long prop = CameraControl_Exposure; prop <= CameraControl_Focus; ++prop) {
        if (SUCCEEDED(pCamControl->GetRange(prop, &minValue, &maxValue, &step, &defaultValue, &flags))) {
                std::cout << "Property " << prop << ": Min = " << minValue
                << ", Max = " << maxValue
                << ", Default = " << defaultValue << "\n";
        }
    }
}

bool CameraManager::isOpened() const {
    return opened;
}

const std::vector<unsigned char>& CameraManager::getFrameData() const {
    return frameData;
}

void CameraManager::initializeGraph() {
    CComPtr<ICreateDevEnum> pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
    if (FAILED(hr)) {
        std::cerr << "Failed to create system device enumerator. HRESULT: " << hr << "\n";
        return;
    }

    CComPtr<IEnumMoniker> pEnum;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_FALSE) {
        std::cerr << "No video capture devices found.\n";
        return;
    }

    CComPtr<IMoniker> pMoniker;
    for (int i = 0; i <= cameraIndex && pEnum->Next(1, &pMoniker, nullptr) == S_OK; ++i) {
        if (i == cameraIndex) {
            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, reinterpret_cast<void**>(&pVideoCapture));
            if (FAILED(hr)) {
                std::cerr << "Failed to bind to video capture filter. HRESULT: " << hr << "\n";
                return;
            }

            hr = pGraph->AddFilter(pVideoCapture, L"Video Capture");
            if (FAILED(hr)) {
                std::cerr << "Failed to add video capture filter to the graph. HRESULT: " << hr << "\n";
                return;
            }

            hr = pVideoCapture->QueryInterface(IID_PPV_ARGS(&pCamControl));
            if (FAILED(hr)) {
                std::cerr << "Failed to get IAMCameraControl interface. HRESULT: " << hr << "\n";
                return;
            }

            hr = pVideoCapture->QueryInterface(IID_PPV_ARGS(&pProcAmp));
            if (FAILED(hr)) {
                std::cerr << "Failed to get IAMVideoProcAmp interface. HRESULT: " << hr << "\n";
                return;
            }
        }
    }
}
