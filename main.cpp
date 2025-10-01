#include "tinygui.h"
#include <cstdio>

int main() {
    if (!tinygui::init(800, 600, "TinyGUI Enhanced Demo")) return -1;

    // Char callback for typing (printable characters inserted at caret)
    glfwSetCharCallback(tinygui::ctx.window, [](GLFWwindow*, unsigned int codepoint){
        tinygui::onChar(codepoint);
    });

    // Key callback for navigation, selection, delete/backspace
    glfwSetKeyCallback(tinygui::ctx.window, [](GLFWwindow*, int key, int scancode, int action, int mods){
        tinygui::onKey(key, scancode, action, mods);
    });
    
    // Demo state variables
    static bool checkboxValue = false;
    static float sliderValue = 0.5f;
    static float progressValue = 0.0f;
    
    // Input field states
    static tinygui::InputState input1;
    static tinygui::InputState input2;

    while (!tinygui::windowShouldClose()) {
        tinygui::pollEvents();
        tinygui::beginFrame();
        
        // Easy menu system - handles everything automatically!
        int menuResult = tinygui::easyMenuBar();
        if (menuResult >= 0) {
            printf("Menu item selected: %d\n", menuResult);
            // Handle menu actions:
            // File: 100-104, Edit: 200-205, View: 300-302, Help: 400-401
            if (menuResult == 104) { // Exit
                break; // Exit the main loop
            }
        }
        
        // Adjust content position for menu bar
        float menuOffset = tinygui::getMenuBarHeight() + 10;
        
        // Manual layout demo
        tinygui::label(50, menuOffset, "TinyGUI Enhanced - With Menu Bar", 3.0f, tinygui::COLOR_YELLOW);
        
        if (tinygui::button(50, menuOffset + 25, 120, 40, "Click Me!"))
            printf("Button clicked!\n");
            
        tinygui::input(180, menuOffset + 25, 200, 40, input1, "Enter text here...");
        
        tinygui::checkbox(50, menuOffset + 85, 20, "Enable feature", checkboxValue);
        
        tinygui::slider(50, menuOffset + 125, 200, 20, sliderValue, 0.0f, 1.0f);
        
        // Update progress based on slider
        progressValue = sliderValue;
        tinygui::progressBar(50, menuOffset + 165, 200, 15, progressValue, tinygui::COLOR_GREEN);
        
        // Layout-based demo
        tinygui::beginLayout(400, menuOffset + 25, true, 10);
        
        tinygui::labelLayout("Auto Layout Demo:", 2.5f, tinygui::COLOR_CYAN);
        tinygui::layoutSpacing(5);
        
        if (tinygui::buttonLayout(150, 35, "Layout Button"))
            printf("Layout button clicked!\n");
        
        tinygui::inputLayout(200, 35, input2, "Layout input...");
        
        static bool layoutCheck = true;
        tinygui::checkboxLayout(18, "Layout checkbox", layoutCheck);
        
        static float layoutSlider = 0.3f;
        tinygui::sliderLayout(180, 20, layoutSlider, 0.0f, 2.0f);
        
        // Display all values in GUI
        char valueText[200];
        snprintf(valueText, sizeof(valueText), "Slider: %.2f, Checkbox: %s", 
                sliderValue, checkboxValue ? "ON" : "OFF");
        tinygui::label(50, menuOffset + 215, valueText, 2.0f, tinygui::THEME_TEXT_DIM);
        
        // Display input field contents
        char inputDisplayText[300];
        snprintf(inputDisplayText, sizeof(inputDisplayText), "Input1: '%s'", input1.text);
        tinygui::label(50, menuOffset + 245, inputDisplayText, 2.0f, tinygui::COLOR_CYAN);
        
        snprintf(inputDisplayText, sizeof(inputDisplayText), "Input2: '%s'", input2.text);
        tinygui::label(50, menuOffset + 275, inputDisplayText, 2.0f, tinygui::COLOR_CYAN);

        tinygui::endFrame(); // Automatically handles dropdown menus!
    }

    return 0;
}
