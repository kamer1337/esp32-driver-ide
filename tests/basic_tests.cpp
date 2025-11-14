#include <iostream>
#include <cassert>
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"

using namespace esp32_ide;

void test_text_editor() {
    std::cout << "Testing TextEditor..." << std::endl;
    
    TextEditor editor;
    editor.SetText("void setup() {}");
    assert(!editor.GetText().empty());
    assert(editor.GetLineCount() == 1);
    
    std::cout << "  ✓ TextEditor tests passed" << std::endl;
}

void test_syntax_highlighter() {
    std::cout << "Testing SyntaxHighlighter..." << std::endl;
    
    SyntaxHighlighter highlighter;
    std::string code = "int main() { return 0; }";
    auto tokens = highlighter.Tokenize(code);
    assert(!tokens.empty());
    
    std::cout << "  ✓ SyntaxHighlighter tests passed" << std::endl;
}

void test_file_manager() {
    std::cout << "Testing FileManager..." << std::endl;
    
    FileManager fm;
    fm.CreateFile("test.ino");
    fm.SetFileContent("test.ino", "void setup() {}");
    assert(fm.GetFileContent("test.ino") == "void setup() {}");
    
    std::cout << "  ✓ FileManager tests passed" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ESP32 Driver IDE - Basic Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        test_text_editor();
        test_syntax_highlighter();
        test_file_manager();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "✓ ALL TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
