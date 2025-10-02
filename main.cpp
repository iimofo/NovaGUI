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
    
    // Tab systems
    static int activeTab = 0; // Horizontal tabs
    static int activeSideTab = 0; // Vertical sidebar tabs
    static const char* tabNames[] = {"General", "Advanced", "Images", "Lists"};
    static const char* sideTabNames[] = {"Home", "Settings", "Projects", "Tools", "Help"};
    
    // List and dropdown demo
    static const char* listItems[] = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
    static int selectedListItem = 0;
    static int selectedDropdownItem = -1;
    
    // Load real images
    static bool imagesLoaded = false;
    if (!imagesLoaded) {
        // Try to load real image files
        if (!tinygui::loadImageFromFile("test1", "test1.jpg")) {
            // Fallback to test pattern if file doesn't exist
            tinygui::createTestImage("test1", 64);
        }
        if (!tinygui::loadImageFromFile("test2", "test2.jpg")) {
            // Fallback to test pattern if file doesn't exist
            tinygui::createTestImage("test2", 64);
        }
        // You can load more images here:
        // tinygui::loadImageFromFile("mylogo", "logo.png");
        // tinygui::loadImageFromFile("icon", "icon.bmp");
        imagesLoaded = true;
    }

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
        
        // Adjust content position for menu bar and sidebar
        float menuOffset = tinygui::getMenuBarHeight();
        float sidebarWidth = tinygui::getSidebarWidth();
        
        // Draw vertical sidebar
        int sideTabClicked = tinygui::verticalTabBar(0, menuOffset, sidebarWidth, sideTabNames, 5, activeSideTab);
        
        // Main content area (offset by sidebar width)
        float contentStartX = sidebarWidth + 10;
        
        // Main content area based on sidebar selection
        float contentY = menuOffset + 20;
        float contentH = 400;
        
        if (activeSideTab == 0) { // Home tab - Simple welcome screen
            tinygui::label(contentStartX, contentY, "Welcome to NovaGUI!", 4.0f, tinygui::COLOR_YELLOW);
            tinygui::label(contentStartX, contentY + 60, "A lightweight C++ GUI library built with OpenGL", 2.0f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 90, "Features:", 2.5f, tinygui::COLOR_CYAN);
            tinygui::label(contentStartX, contentY + 120, "• Easy-to-use widgets (buttons, inputs, sliders, etc.)", 1.8f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 140, "• Real image loading support", 1.8f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 160, "• Vertical and horizontal tab systems", 1.8f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 180, "• Modal dialogs and menus", 1.8f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 200, "• Layout management system", 1.8f, tinygui::THEME_TEXT);
            
            if (tinygui::autoButton(contentStartX, contentY + 240, "Get Started with Settings", 12.0f)) {
                activeSideTab = 1; // Switch to Settings tab
            }
            
        } else if (activeSideTab == 1) { // Settings tab - All the detailed functionality
            tinygui::label(contentStartX, contentY, "Settings & Configuration", 3.0f, tinygui::COLOR_CYAN);
            
            // Horizontal tab bar within Settings
            float tabY = contentY + 50;
            int tabClicked = tinygui::tabBar(contentStartX, tabY, 600, 30, tabNames, 4, activeTab);
            
            // Tab content area
            float subContentY = tabY + 35;
            float subContentH = 300;
            tinygui::beginTabContent(contentStartX, subContentY, 600, subContentH);
            
            // Tab content based on active tab
            if (activeTab == 0) { // General tab
                tinygui::label(contentStartX + 10, subContentY + 10, "General Settings", 2.5f, tinygui::COLOR_CYAN);
                
                // Option 1: Manual button sizing (old way, but now with proper sizes)
                float widgetX = contentStartX + 10;
                if (tinygui::button(widgetX, subContentY + 45, 130, 35, "Alert Dialog"))
                    tinygui::alert("Information", "This is an alert dialog!");
                    
                if (tinygui::button(widgetX + 140, subContentY + 45, 150, 35, "Confirm Dialog"))
                    if (tinygui::confirm("Confirm", "Do you want to proceed?"))
                        printf("User confirmed!\n");
                
                // Option 2: Auto-sizing buttons (new way - automatically fits text + padding)
                // if (tinygui::autoButton(widgetX, subContentY + 45, "Alert Dialog"))
                //     tinygui::alert("Information", "This is an alert dialog!");
                // if (tinygui::autoButton(widgetX + 140, subContentY + 45, "Confirm Dialog"))
                //     if (tinygui::confirm("Confirm", "Do you want to proceed?"))
                //         printf("User confirmed!\n");
                
                tinygui::input(widgetX, subContentY + 100, 200, 35, input1, "Enter your name...");
                
                tinygui::checkbox(widgetX, subContentY + 150, 20, "Enable notifications", checkboxValue);
                
                tinygui::label(widgetX, subContentY + 190, "Volume:");
                tinygui::slider(widgetX, subContentY + 210, 200, 20, sliderValue, 0.0f, 1.0f);
                
                progressValue = sliderValue;
                tinygui::progressBar(widgetX, subContentY + 250, 200, 15, progressValue, tinygui::COLOR_GREEN);
            
            } else if (activeTab == 1) { // Advanced tab
                tinygui::label(contentStartX + 10, subContentY + 10, "Advanced Features", 2.5f, tinygui::COLOR_CYAN);
                
                tinygui::beginLayout(contentStartX + 10, subContentY + 50, true, 10);
            tinygui::labelLayout("Auto Layout Example:");
            
            if (tinygui::buttonLayout(150, 35, "Save Settings"))
                printf("Settings saved!\n");
                
            tinygui::inputLayout(200, 35, input2, "Auto-positioned input...");
            
            static bool autoCheck = true;
            tinygui::checkboxLayout(18, "Auto-save enabled", autoCheck);
            
            static float autoSlider = 0.3f;
            tinygui::sliderLayout(180, 20, autoSlider, 0.0f, 2.0f);
            
            } else if (activeTab == 2) { // Images tab
                float widgetX = contentStartX + 10;
                tinygui::label(widgetX, subContentY + 10, "Image Display", 2.5f, tinygui::COLOR_CYAN);
                
                tinygui::label(widgetX, subContentY + 50, "Test Images:");
                tinygui::image(widgetX, subContentY + 80, 64, 64, "test1");
                tinygui::image(widgetX + 80, subContentY + 80, 80, 80, "test2");
                tinygui::image(widgetX + 180, subContentY + 80, 100, 100, "nonexistent"); // Shows error placeholder
                
                tinygui::label(widgetX, subContentY + 200, "Real images loaded from test1.jpg and test2.jpg!", 1.8f, tinygui::COLOR_GREEN);
            
            } else if (activeTab == 3) { // Lists tab
                float widgetX = contentStartX + 10;
                tinygui::label(widgetX, subContentY + 10, "Lists & Dropdowns", 2.5f, tinygui::COLOR_CYAN);
                
                tinygui::label(widgetX, subContentY + 50, "List Box:");
                int listClicked = tinygui::listBox(widgetX, subContentY + 75, 150, 120, listItems, 5, selectedListItem);
                if (listClicked >= 0) {
                    printf("Selected list item: %s\n", listItems[listClicked]);
                }
                
                tinygui::label(widgetX + 180, subContentY + 50, "Dropdown:");
                int dropdownClicked = tinygui::simpleDropdown(widgetX + 180, subContentY + 75, 150, 30, listItems, 5, selectedDropdownItem);
                if (dropdownClicked >= 0) {
                    printf("Selected dropdown item: %s\n", listItems[dropdownClicked]);
                }
                
                // Show selected items
                char selectionText[200];
                snprintf(selectionText, sizeof(selectionText), "List: %s, Dropdown: %s", 
                        selectedListItem >= 0 ? listItems[selectedListItem] : "None",
                        selectedDropdownItem >= 0 ? listItems[selectedDropdownItem] : "None");
                tinygui::label(widgetX, subContentY + 250, selectionText, 1.8f, tinygui::COLOR_CYAN);
            }
            
            tinygui::endTabContent();
            
        } else if (activeSideTab == 2) { // Projects tab
            tinygui::label(contentStartX, contentY, "Projects", 3.0f, tinygui::COLOR_CYAN);
            tinygui::label(contentStartX, contentY + 50, "Project management features will be added here.", 2.0f, tinygui::THEME_TEXT);
            
        } else if (activeSideTab == 3) { // Tools tab
            tinygui::label(contentStartX, contentY, "Tools", 3.0f, tinygui::COLOR_CYAN);
            tinygui::label(contentStartX, contentY + 50, "Development tools and utilities will be added here.", 2.0f, tinygui::THEME_TEXT);
            
        } else if (activeSideTab == 4) { // Help tab
            tinygui::label(contentStartX, contentY, "Help & Documentation", 3.0f, tinygui::COLOR_CYAN);
            tinygui::label(contentStartX, contentY + 50, "NovaGUI Documentation", 2.5f, tinygui::COLOR_YELLOW);
            tinygui::label(contentStartX, contentY + 80, "Quick Start Guide:", 2.0f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 110, "1. Include tinygui.h in your project", 1.8f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 130, "2. Call tinygui::init(width, height, title)", 1.8f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 150, "3. Create your main loop with beginFrame/endFrame", 1.8f, tinygui::THEME_TEXT);
            tinygui::label(contentStartX, contentY + 170, "4. Add widgets like buttons, inputs, labels, etc.", 1.8f, tinygui::THEME_TEXT);
            
            if (tinygui::autoButton(contentStartX, contentY + 210, "Visit GitHub Repository", 10.0f)) {
                printf("Opening GitHub repository...\n");
            }
        }
        
        // Status information at bottom
        char statusText[400];
        if (activeSideTab == 1) {
            snprintf(statusText, sizeof(statusText), "Input1: '%s' | Input2: '%s' | Slider: %.2f | Settings Tab: %s | Sidebar: %s", 
                    input1.text, input2.text, sliderValue, tabNames[activeTab], sideTabNames[activeSideTab]);
        } else {
            snprintf(statusText, sizeof(statusText), "Sidebar: %s", sideTabNames[activeSideTab]);
        }
        tinygui::label(contentStartX, contentY + contentH + 20, statusText, 1.5f, tinygui::THEME_TEXT_DIM);

        tinygui::endFrame(); // Automatically handles dropdown menus!
    }

    return 0;
}
