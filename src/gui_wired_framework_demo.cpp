#include "gui/gui_wired_framework.h"
#include <iostream>
#include <memory>

/**
 * @brief Demo application showing how to use the GUI Wired Framework
 * 
 * This demonstrates the proper way to wire a GUI window to the backend
 * using the GuiWiredFramework class.
 * 
 * The framework provides:
 * - Clean separation between frontend (GUI) and backend (business logic)
 * - Event-based communication through callbacks
 * - Widget bindings for easy action mapping
 * - State updates for UI refresh
 */
int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "ESP32 Driver IDE - GUI Wired Framework Demo\n";
    std::cout << "========================================\n\n";
    
    try {
        // Create the wired framework
        esp32_ide::gui::GuiWiredFramework framework;
        
        // Option 1: Use default adapters (recommended for production)
        // The framework will create default FrontendAdapter and BackendAdapter
        // that wrap EnhancedGuiWindow and BackendFramework respectively
        
        // Option 2: Use custom adapters (for testing or custom implementations)
        // auto frontend = std::make_shared<MyCustomFrontend>();
        // auto backend = std::make_shared<MyCustomBackend>();
        // framework.SetFrontend(frontend);
        // framework.SetBackend(backend);
        
        // Initialize the framework
        std::cout << "Initializing GUI Wired Framework...\n";
        if (!framework.Initialize(1280, 800)) {
            std::cerr << "Failed to initialize framework\n";
            return 1;
        }
        
        // Add custom widget bindings
        std::cout << "Setting up custom widget bindings...\n";
        
        // Bind a custom button to a callback
        framework.BindWidget("btn_custom_action", []() {
            std::cout << "Custom action executed!\n";
        });
        
        // Bind a widget to a standard action
        framework.BindWidget("btn_my_compile", esp32_ide::gui::GuiAction::BUILD_VERIFY);
        
        // Register custom action handlers
        framework.RegisterActionHandler(esp32_ide::gui::GuiAction::HELP_ABOUT, 
            [](const esp32_ide::gui::GuiEvent& event) {
                std::cout << "\n";
                std::cout << "========================================\n";
                std::cout << "ESP32 Driver IDE v2.0.0\n";
                std::cout << "----------------------------------------\n";
                std::cout << "A modern C++ IDE for ESP32 development\n";
                std::cout << "with integrated device library and\n";
                std::cout << "GUI wired framework for clean separation\n";
                std::cout << "of concerns.\n";
                std::cout << "========================================\n\n";
            });
        
        // Demonstrate programmatic action triggering
        std::cout << "\nDemonstrating action triggers:\n";
        std::cout << "----------------------------------------\n";
        
        // Create a new file
        framework.TriggerAction(esp32_ide::gui::GuiAction::FILE_NEW, "demo.ino");
        
        // Show About dialog
        framework.TriggerAction(esp32_ide::gui::GuiAction::HELP_ABOUT);
        
        // Toggle panels
        std::cout << "Toggling panels...\n";
        framework.ToggleConsole();
        framework.ToggleTerminal();
        framework.ToggleDevices();
        
        // Add a device
        std::cout << "Adding device...\n";
        esp32_ide::gui::GuiEvent addDeviceEvent(esp32_ide::gui::GuiAction::DEVICE_ADD, "", "dht22");
        framework.TriggerAction(addDeviceEvent);
        
        // Simulate a build
        std::cout << "\nSimulating build process...\n";
        framework.TriggerAction(esp32_ide::gui::GuiAction::BUILD_VERIFY);
        
        // Note: In a real application, you would call framework.Run() here
        // which enters the main event loop:
        // framework.Run();
        
        // For this demo, we just demonstrate the framework capabilities
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "Demo complete! The GUI Wired Framework\n";
        std::cout << "is ready for use in your application.\n";
        std::cout << "========================================\n";
        
        // Cleanup
        framework.Shutdown();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
