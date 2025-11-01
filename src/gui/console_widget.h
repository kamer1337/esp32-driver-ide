#ifndef CONSOLE_WIDGET_H
#define CONSOLE_WIDGET_H

#include <string>
#include <vector>

namespace esp32_ide {
namespace gui {

/**
 * @brief Console widget for displaying output messages
 */
class ConsoleWidget {
public:
    enum class MessageType {
        NORMAL,
        ERROR,
        WARNING,
        SUCCESS,
        INFO
    };
    
    struct Message {
        std::string content;
        MessageType type;
        long long timestamp;
    };
    
    ConsoleWidget();
    ~ConsoleWidget();
    
    // Message management
    void AddMessage(const std::string& content, MessageType type = MessageType::NORMAL);
    void Clear();
    std::vector<Message> GetMessages() const;
    
    // Display
    void Print() const;
    void PrintColored() const;
    
private:
    std::vector<Message> messages_;
    
    static std::string GetColorCode(MessageType type);
    static const char* GetResetCode();
};

} // namespace gui
} // namespace esp32_ide

#endif // CONSOLE_WIDGET_H
