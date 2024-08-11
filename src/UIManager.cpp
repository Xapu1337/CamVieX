#include "UIManager.h"
#include "StringConversion.h"  // Include the header where the wstringToString function is declared
#include "imgui.h"
#include <iostream>
#include "WebcamController.h"

// Initialize the WebcamController with Direct3D device and context
UIManager::UIManager(ID3D11Device* device, ID3D11DeviceContext* context)
    : webcam(device, context) {
    CreateModals();
}

void UIManager::CreateModals() {
    // This method is kept in case you want to initialize or preload modals if necessary.
}

void UIManager::Render() {
    // Top Menu
    ImGui::BeginMainMenuBar();
    if (ImGui::MenuItem("Exit")) {
        std::cerr << "[UIManager] Exit selected" << std::endl;
    }
    if (ImGui::MenuItem("Select Camera")) {
        std::cerr << "[UIManager] 'Select Camera' selected" << std::endl;
        auto selectCameraModal = std::make_unique<ImGuiModaler>("SelectCameraModal", [this]() {
            ImGui::Text("Select Camera");

            // Get the list of available cameras
            auto devices = webcam.ListAvailableCameras();
            if (devices.empty()) {
                ImGui::Text("No cameras found.");
                return;
            }

            static int currentCameraIndex = 0;
            std::vector<const char*> cameraNames;

            // Convert std::wstring to std::string and store it in cameraNames
            std::vector<std::string> cameraNamesStrings;
            for (const auto& cam : devices) {
                cameraNamesStrings.push_back(wstringToString(cam)); // Use the conversion function
            }

            // Convert std::string to const char* and store it in cameraNames for ImGui::Combo
            for (const auto& camName : cameraNamesStrings) {
                cameraNames.push_back(camName.c_str());
            }

            // Create a combo box for camera selection
            ImGui::Combo("Cameras", &currentCameraIndex, cameraNames.data(), static_cast<int>(cameraNames.size()));

            // Place "Select" and "Close" buttons in a group
            ImGui::BeginGroup();
            if (ImGui::Button("Select") && currentCameraIndex >= 0) {
                if (!SUCCEEDED(webcam.Initialize(currentCameraIndex))) {
                    std::cerr << "Failed to initialize the webcam." << std::endl;
                }
                else {
                    std::cout << "Webcam initialized." << std::endl;
                    webcam.StartCapture();  // Start capturing immediately
                }
                ImGuiModaler::CloseCurrentModal();
            }

            ImGui::SameLine(); // Place buttons inline
            if (ImGui::Button("Close")) {
                ImGuiModaler::CloseCurrentModal();
            }
            ImGui::EndGroup();

            // Provide feedback if no camera is selected and user tries to press "Select"
            if (currentCameraIndex == -1) {
                ImGui::Text("Please select a camera.");
            }
            });
        selectCameraModal->SetBackdrop(true, 0.9f);  // Set backdrop properties
        ImGuiModaler::ShowModal(std::move(selectCameraModal));
    }
    if (ImGui::MenuItem("Settings")) {
        std::cerr << "[UIManager] 'Settings' selected" << std::endl;
        auto settingsModal = std::make_unique<ImGuiModaler>("SettingsModal", [this]() {
            ImGui::Text("Settings");
            // Implement settings UI here
            });
        settingsModal->SetBackdrop(true, 0.9f);  // Set backdrop properties
        ImGuiModaler::ShowModal(std::move(settingsModal));
    }
    ImGui::EndMainMenuBar();

    // Handle Overlays (render modals)
    HandleOverlays();

    // Left Pane - Camera Feed
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
    ImGui::Begin("Camera Feed", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (ID3D11ShaderResourceView* tex = webcam.GetFrameTexture()) {
        ImGui::Image(tex, ImVec2(640, 480)); // Adjust size as necessary
    }
    else {
        ImGui::Text("No frame available");
    }


    ImGui::End();

    // Right Pane - Sidebar
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Implement settings UI here

    ImGui::End();
}

void UIManager::HandleOverlays() {
    // Render modals if they are open
    ImGuiModaler::RenderActiveModal();
}
