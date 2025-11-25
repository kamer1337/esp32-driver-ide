/**
 * @file terminal_main.cpp
 * @brief Entry point for ESP32 Driver IDE Terminal Mode
 * 
 * This provides a CLI-based alternative to the GUI version,
 * inspired by Arduino CLI.
 */

#include "terminal/terminal_mode.h"

int main(int argc, char* argv[]) {
    return esp32_ide::TerminalMain(argc, argv);
}
