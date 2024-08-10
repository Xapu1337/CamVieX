#pragma once

#include <dshow.h>
#include <string>
#include <vector>
#include <memory>
#include <atlbase.h>

class CameraManager {
public:
    CameraManager();
    ~CameraManager();

    bool open(int cameraIndex);
    void close();
    bool captureFrame();
    void setProperty(long property, long value);
    long getProperty(long property);
    void dumpCameraInfo(); // Debugger for camera properties

    bool isOpened() const;

    const std::vector<unsigned char>& getFrameData() const;

private:
    void initializeGraph();

    CComPtr<IGraphBuilder> pGraph;
    CComPtr<ICaptureGraphBuilder2> pBuilder;
    CComPtr<IMediaControl> pControl;
    CComPtr<IBaseFilter> pVideoCapture;
    CComPtr<IAMCameraControl> pCamControl;
    CComPtr<IAMVideoProcAmp> pProcAmp;

    int cameraIndex = -1;
    bool opened = false;
    std::vector<unsigned char> frameData;
};
