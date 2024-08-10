#pragma once

#include <memory>
#include "CameraManager.h"

class UIManager {
public:
    UIManager(std::shared_ptr<CameraManager> cameraManager);

    // Render the UI
    void render();

private:
    void handleOverlays();

    std::shared_ptr<CameraManager> cameraManager;

    // Popup state flags
    bool showSelectCameraPopup = false;
    bool showSettingsPopup = false;
};
