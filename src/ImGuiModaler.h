#pragma once

#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <string>
#include <atomic>
#include <iostream>
#include "imgui.h"

class ImGuiModaler {
public:
    ImGuiModaler(const std::string& id, std::function<void()> contentFunc);

    static void ShowModal(std::unique_ptr<ImGuiModaler> modal);
    static void CloseCurrentModal();
    static void RenderActiveModal();

    void SetBackdrop(bool enabled = true, float alpha = 0.5f);

private:
    void RenderBackdrop();
    void RenderModal();

    std::string id;
    std::function<void()> contentFunc;
    std::atomic<bool> isOpen{ false };
    bool backdropEnabled = true;
    float backdropAlpha = 0.5f;

    static std::queue<std::unique_ptr<ImGuiModaler>> modalQueue;
    static std::unique_ptr<ImGuiModaler> activeModal;
    static std::mutex queueMutex;
};
