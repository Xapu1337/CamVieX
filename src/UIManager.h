#pragma once

#include <memory>
#include "WebcamController.h"
#include "imgui.h"
#include "ImGuiModaler.h"  // Assuming you have a modal handling utility like ImGuiModaler

class UIManager {
public:
    // Constructor that accepts Direct3D device and context for webcam controller
    UIManager(ID3D11Device* device, ID3D11DeviceContext* context);

    // Main render function for the UI
    void Render();

private:
    // Webcam controller instance
    WebcamController webcam;

    // Function to create modals (e.g., for camera selection)
    void CreateModals();

    // Function to handle overlays such as modals
    void HandleOverlays();
};

