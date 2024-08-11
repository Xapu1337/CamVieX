#include "ImGuiModaler.h"

// Static members initialization
std::queue<std::unique_ptr<ImGuiModaler>> ImGuiModaler::modalQueue;
std::unique_ptr<ImGuiModaler> ImGuiModaler::activeModal = nullptr;

ImGuiModaler::ImGuiModaler(const std::string& id, std::function<void()> contentFunc)
    : id(id), contentFunc(std::move(contentFunc)) {
    if (id.empty()) {
        throw std::invalid_argument("Modal ID cannot be empty");
    }
    if (!this->contentFunc) {
        throw std::invalid_argument("Content function cannot be null");
    }
    std::cerr << "[ImGuiModaler] Created modal with ID: " << id << std::endl;
}

void ImGuiModaler::ShowModal(std::unique_ptr<ImGuiModaler> modal) {
    std::cerr << "[ImGuiModaler] ShowModal called for ID: " << modal->id << std::endl;

    if (activeModal && activeModal->isOpen.load()) {
        std::cerr << "[ImGuiModaler] Another modal is active, adding to queue: " << modal->id << std::endl;
        modalQueue.push(std::move(modal));
    }
    else {
        std::cerr << "[ImGuiModaler] No active modal, displaying: " << modal->id << std::endl;
        activeModal = std::move(modal);
        activeModal->isOpen.store(true);  // Set the modal as open
    }
}

void ImGuiModaler::CloseCurrentModal() {
    std::cerr << "[ImGuiModaler] CloseCurrentModal called" << std::endl;

    if (!activeModal) {
        std::cerr << "[ImGuiModaler] No active modal to close" << std::endl;
        return;
    }

    if (activeModal->isOpen.load()) {
        std::cerr << "[ImGuiModaler] Closing modal with ID: " << activeModal->id << std::endl;
        activeModal->isOpen.store(false);  // Set the modal as closed
        activeModal.reset();  // Destroy the current modal

        // Check if there's another modal in the queue
        if (!modalQueue.empty()) {
            activeModal = std::move(modalQueue.front());
            modalQueue.pop();
            std::cerr << "[ImGuiModaler] Opening next modal from queue with ID: " << activeModal->id << std::endl;
            activeModal->isOpen.store(true);  // Open the next modal
        }
        else {
            std::cerr << "[ImGuiModaler] No more modals in the queue." << std::endl;
            activeModal = nullptr;
        }
    }
    else {
        std::cerr << "[ImGuiModaler] Modal was already closed." << std::endl;
    }
}

void ImGuiModaler::RenderActiveModal() {
    if (!activeModal || !activeModal->isOpen.load()) {
        std::cerr << "[ImGuiModaler] No active modal to render" << std::endl;
        return;
    }

    std::cerr << "[ImGuiModaler] Rendering active modal with ID: " << activeModal->id << std::endl;
    if (activeModal->backdropEnabled) {
        activeModal->RenderBackdrop();
    }
    activeModal->RenderModal();
}

void ImGuiModaler::SetBackdrop(bool enabled, float alpha) {
    std::cerr << "[ImGuiModaler] SetBackdrop called for ID: " << id << std::endl;
    backdropEnabled = enabled;
    backdropAlpha = alpha;
}

void ImGuiModaler::RenderBackdrop() {
    std::cerr << "[ImGuiModaler] Rendering backdrop for modal with ID: " << id << std::endl;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, backdropAlpha));
    ImGuiWindowFlags backdropFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
    backdropFlags |= ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("##Backdrop", nullptr, backdropFlags);
    ImGui::CaptureMouseFromApp(true);
    ImGui::CaptureKeyboardFromApp(true);
    ImGui::End();
    ImGui::PopStyleColor();
}

void ImGuiModaler::RenderModal() {
    std::cerr << "[ImGuiModaler] Rendering modal with ID: " << id << std::endl;

    ImVec2 windowSize(400, 300);
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - windowSize.x) * 0.5f, (ImGui::GetIO().DisplaySize.y - windowSize.y) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowFocus();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    flags |= ImGuiWindowFlags_NoNavFocus;

    if (ImGui::Begin(id.c_str(), nullptr, flags)) {
        contentFunc();

        ImGui::SetWindowFocus();

        if (ImGui::Button("Close")) {
            std::cerr << "[ImGuiModaler] Close button clicked for modal with ID: " << id << std::endl;
            CloseCurrentModal();  // Close modal safely
        }
        ImGui::End();
    }
}
