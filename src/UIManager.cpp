#include "UIManager.h"
#include "imgui.h"
#include <iostream>

UIManager::UIManager(std::shared_ptr<CameraManager> cameraManager)
    : cameraManager(cameraManager) {}

void UIManager::render() {
    std::cerr << "[UIManager] Starting render" << std::endl;

    // Top Menu
    ImGui::BeginMainMenuBar();
    if (ImGui::MenuItem("Exit")) {
        std::cerr << "[UIManager] Exit selected" << std::endl;
        PostQuitMessage(0);
    }
    if (ImGui::MenuItem("Select Camera")) {
        std::cerr << "[UIManager] 'Select Camera' selected" << std::endl;
        showSelectCameraPopup = true;
        ImGui::OpenPopup("Select Camera");
    }
    if (ImGui::MenuItem("Settings")) {
        std::cerr << "[UIManager] 'Settings' selected" << std::endl;
        showSettingsPopup = true;
        ImGui::OpenPopup("Settings");
    }
    ImGui::EndMainMenuBar();

    // Handle Popups
    handleOverlays();

    // Left Pane - Camera Feed
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
    ImGui::Begin("Camera Feed", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    if (cameraManager->isOpened()) {
        ImGui::Text("Camera Feed Display (Placeholder)");
    }
    else {
        ImGui::Text("No Camera Selected.");
    }
    ImGui::End();

    // Right Pane - Sidebar
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    if (cameraManager->isOpened()) {
        int exposure = static_cast<int>(cameraManager->getProperty(CameraControl_Exposure));
        if (ImGui::SliderInt("Exposure", &exposure, -10, 10)) {
            cameraManager->setProperty(CameraControl_Exposure, exposure);
        }
        cameraManager->dumpCameraInfo();
    }
    else {
        ImGui::Text("No camera opened.");
    }
    ImGui::End();

    std::cerr << "[UIManager] End render" << std::endl;
}

void UIManager::handleOverlays() {
    if (ImGui::BeginPopupModal("Select Camera", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select Camera (this is a placeholder)");
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            showSelectCameraPopup = false;
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Settings (this is a placeholder)");
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            showSettingsPopup = false;
        }
        ImGui::EndPopup();
    }
}
