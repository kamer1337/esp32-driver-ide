#include "gui/console_widget.h"
#include <iostream>
#include <chrono>

namespace esp32_ide {
namespace gui {

ConsoleWidget::ConsoleWidget() = default;

ConsoleWidget::~ConsoleWidget() = default;

void ConsoleWidget::AddMessage(const std::string& content, MessageType type) {
    Message msg;
    msg.content = content;
    msg.type = type;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    messages_.push_back(msg);
}

void ConsoleWidget::Clear() {
    messages_.clear();
}

std::vector<ConsoleWidget::Message> ConsoleWidget::GetMessages() const {
    return messages_;
}

void ConsoleWidget::Print() const {
    for (const auto& msg : messages_) {
        std::cout << msg.content << "\n";
    }
}

void ConsoleWidget::PrintColored() const {
    for (const auto& msg : messages_) {
        std::cout << GetColorCode(msg.type) << msg.content << GetResetCode() << "\n";
    }
}

std::string ConsoleWidget::GetColorCode(MessageType type) {
    switch (type) {
        case MessageType::ERROR:   return "\033[31m"; // Red
        case MessageType::WARNING: return "\033[33m"; // Yellow
        case MessageType::SUCCESS: return "\033[32m"; // Green
        case MessageType::INFO:    return "\033[36m"; // Cyan
        default:                   return "\033[0m";  // Reset
    }
}

const char* ConsoleWidget::GetResetCode() {
    return "\033[0m";
}

} // namespace gui
} // namespace esp32_ide
