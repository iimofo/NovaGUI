#include "tinygui.h"
#include <cstdio>

int main() {
    if (!tinygui::init(800, 600, "My Window")) return -1;

    // Char callback for typing (printable characters inserted at caret)
    glfwSetCharCallback(tinygui::ctx.window, [](GLFWwindow*, unsigned int codepoint){
        tinygui::onChar(codepoint);
    });

    // Key callback for navigation, selection, delete/backspace
    glfwSetKeyCallback(tinygui::ctx.window, [](GLFWwindow*, int key, int scancode, int action, int mods){
        tinygui::onKey(key, scancode, action, mods);
    });

    while (!tinygui::windowShouldClose()) {
        tinygui::pollEvents();
        tinygui::beginFrame();

        tinygui::label(100, 50, "Type below:", 3.0f);

        if (tinygui::button(100, 100, 120, 40, "Click Me"))
            printf("Button clicked!\n");

        tinygui::input(100, 200, 300, 30, "Enter text here...");

        tinygui::endFrame();
    }

    return 0;
}