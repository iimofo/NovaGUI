# NovaGUI (TinyGUI)

A powerful, easy-to-use C++ GUI library for building desktop applications with minimal code!

## ✨ Features

### 🎨 **Core Widgets**
- **Windows** with minimum size constraints (800×600)
- **Labels** with custom colors and scaling
- **Buttons** with hover effects and click detection
- **Text Input** with cursor, text selection, copy/paste support
- **Checkboxes** with custom text labels
- **Sliders** with customizable ranges
- **Progress Bars** with custom colors

### 📋 **Menu System**
- **Professional Menu Bar** (File, Edit, View, Help)
- **Dropdown Menus** with separators and disabled items
- **Automatic rendering** - no complex setup required
- **Menu item callbacks** with unique IDs

### 🎨 **Theming & Colors**
- **Built-in color system** (RED, GREEN, BLUE, YELLOW, etc.)
- **Professional themes** (dark theme included)
- **Customizable colors** for all widgets
- **Consistent visual styling**

### 📐 **Layout Management**
- **Auto-layout system** - widgets position themselves
- **Manual positioning** for precise control
- **Spacing controls** and alignment helpers
- **Layout-aware widgets** for rapid development

### 🔧 **Developer Features**
- **Single header file** - just include `tinygui.h`
- **No complex setup** - works out of the box
- **Cross-platform** - Windows, macOS, Linux
- **Beginner-friendly** API with extensive examples

---

## 🏁 Quick Start

### 1. **Include the Library**
```cpp
#include "tinygui.h"
#include <cstdio>
```

### 2. **Basic Application Structure**
```cpp
int main() {
    // Initialize window (800x600, minimum size enforced)
    if (!tinygui::init(800, 600, "My App")) return -1;

    // Setup input callbacks
    glfwSetCharCallback(tinygui::ctx.window, [](GLFWwindow*, unsigned int codepoint){
        tinygui::onChar(codepoint);
    });
    glfwSetKeyCallback(tinygui::ctx.window, [](GLFWwindow*, int key, int scancode, int action, int mods){
        tinygui::onKey(key, scancode, action, mods);
    });

    while (!tinygui::windowShouldClose()) {
        tinygui::pollEvents();
        tinygui::beginFrame();

        // Your GUI code here
        tinygui::label(50, 50, "Hello World!");
        if (tinygui::button(50, 100, 120, 40, "Click Me!")) {
            printf("Button clicked!\n");
        }

        tinygui::endFrame();
    }
    return 0;
}
```

---

## 📚 Widget Reference

### 🏷️ **Labels**
```cpp
// Basic label
tinygui::label(x, y, "Text");

// Styled label
tinygui::label(x, y, "Colored Text", 2.0f, tinygui::COLOR_RED);
```

### 🔘 **Buttons**
```cpp
if (tinygui::button(x, y, width, height, "Button Text")) {
    // Button was clicked
}
```

### ✏️ **Text Input**
```cpp
// Create input state (one per input field)
static tinygui::InputState myInput;

// Draw input field
tinygui::input(x, y, width, height, myInput, "Hint text...");

// Access the text
printf("User typed: %s\n", myInput.text);
```

### ☑️ **Checkboxes**
```cpp
static bool checked = false;
tinygui::checkbox(x, y, size, "Enable feature", checked);
```

### 🎚️ **Sliders**
```cpp
static float value = 0.5f;
tinygui::slider(x, y, width, height, value, 0.0f, 1.0f);
```

### 📊 **Progress Bars**
```cpp
float progress = 0.75f; // 75%
tinygui::progressBar(x, y, width, height, progress, tinygui::COLOR_GREEN);
```

### 📋 **Menu System**
```cpp
// Easy menu system (handles everything automatically)
int menuResult = tinygui::easyMenuBar();
if (menuResult == 104) { // File -> Exit
    break; // Close application
}
// Menu IDs: File(100-104), Edit(200-205), View(300-302), Help(400-401)
```

---

## 🎨 **Colors & Theming**

### **Predefined Colors**
```cpp
tinygui::COLOR_WHITE
tinygui::COLOR_BLACK
tinygui::COLOR_RED
tinygui::COLOR_GREEN
tinygui::COLOR_BLUE
tinygui::COLOR_YELLOW
tinygui::COLOR_ORANGE
tinygui::COLOR_PURPLE
tinygui::COLOR_CYAN
```

### **Theme Colors**
```cpp
tinygui::THEME_BG          // Background
tinygui::THEME_BUTTON      // Button normal
tinygui::THEME_BUTTON_HOVER // Button hovered
tinygui::THEME_INPUT       // Input field
tinygui::THEME_TEXT        // Normal text
tinygui::THEME_TEXT_DIM    // Disabled text
```

### **Custom Colors**
```cpp
tinygui::Color myColor(1.0f, 0.5f, 0.2f); // RGB values 0.0-1.0
tinygui::label(x, y, "Custom Color", 2.0f, myColor);
```

---

## 📐 **Layout Management**

### **Auto Layout**
```cpp
// Start vertical layout at position (x, y)
tinygui::beginLayout(100, 50, true, 10); // vertical, 10px spacing

// Widgets position themselves automatically
tinygui::labelLayout("Settings");
tinygui::buttonLayout(150, 35, "Save");
tinygui::inputLayout(200, 35, inputState, "Name...");
tinygui::checkboxLayout(20, "Auto-save", autoSave);
tinygui::sliderLayout(180, 20, volume, 0.0f, 1.0f);

// Add extra spacing
tinygui::layoutSpacing(20);
```

### **Manual Positioning**
```cpp
// Position widgets manually
tinygui::button(50, 100, 120, 40, "Button 1");
tinygui::button(200, 100, 120, 40, "Button 2");
```

---

## 🔧 **Build Instructions**

### **Prerequisites**
1. **C++ Compiler** (GCC/MinGW, Visual Studio, or Clang)
2. **GLFW Library** - Install via:
   - **Windows (Conda)**: `conda install -c conda-forge glfw`
   - **Windows (vcpkg)**: `vcpkg install glfw3`
   - **Linux**: `sudo apt-get install libglfw3-dev`
   - **macOS**: `brew install glfw`

### **Build Commands**

#### **Windows (MinGW with Conda)**
```bash
g++ -o myapp.exe main.cpp \
    -I"C:\Users\[User]\miniconda3\Library\include" \
    -L"C:\Users\[User]\miniconda3\Library\lib" \
    -lglfw3dll -lopengl32 -lglu32 -lgdi32
```

#### **Windows (Manual GLFW)**
```bash
g++ -o myapp.exe main.cpp -std=c++17 \
    -I"C:\glfw\include" \
    -L"C:\glfw\lib-mingw-w64" \
    -lglfw3 -lopengl32 -lgdi32 \
    -static-libgcc -static-libstdc++
```

#### **Linux**
```bash
g++ -o myapp main.cpp -lglfw -lGL -lX11 -lpthread -ldl
```

#### **macOS**
```bash
g++ -o myapp main.cpp -lglfw -framework OpenGL -framework Cocoa -framework IOKit
```

---

## 📁 **File Structure**
```
YourProject/
├── tinygui.h          # Main library (single header)
├── stb_easy_font.h    # Font rendering (included)
├── main.cpp           # Your application
└── README.md
```

---

## 🎯 **Complete Example**

```cpp
#include "tinygui.h"
#include <cstdio>

int main() {
    if (!tinygui::init(800, 600, "My GUI App")) return -1;

    // Input callbacks
    glfwSetCharCallback(tinygui::ctx.window, [](GLFWwindow*, unsigned int c){
        tinygui::onChar(c);
    });
    glfwSetKeyCallback(tinygui::ctx.window, [](GLFWwindow*, int key, int scancode, int action, int mods){
        tinygui::onKey(key, scancode, action, mods);
    });

    // Application state
    static tinygui::InputState nameInput;
    static tinygui::InputState emailInput;
    static bool notifications = true;
    static float volume = 0.7f;

    while (!tinygui::windowShouldClose()) {
        tinygui::pollEvents();
        tinygui::beginFrame();

        // Handle menu
        int menu = tinygui::easyMenuBar();
        if (menu == 104) break; // Exit

        float menuHeight = tinygui::getMenuBarHeight();

        // Manual layout
        tinygui::label(50, menuHeight + 20, "User Settings", 3.0f, tinygui::COLOR_YELLOW);
        
        tinygui::label(50, menuHeight + 60, "Name:");
        tinygui::input(120, menuHeight + 55, 200, 30, nameInput, "Enter your name...");
        
        tinygui::label(50, menuHeight + 100, "Email:");
        tinygui::input(120, menuHeight + 95, 200, 30, emailInput, "Enter email...");
        
        tinygui::checkbox(50, menuHeight + 140, 20, "Enable notifications", notifications);
        
        tinygui::label(50, menuHeight + 180, "Volume:");
        tinygui::slider(120, menuHeight + 180, 200, 20, volume, 0.0f, 1.0f);
        
        tinygui::progressBar(50, menuHeight + 220, 200, 15, volume, tinygui::COLOR_GREEN);
        
        if (tinygui::button(50, menuHeight + 260, 100, 35, "Save Settings")) {
            printf("Name: %s\n", nameInput.text);
            printf("Email: %s\n", emailInput.text);
            printf("Notifications: %s\n", notifications ? "ON" : "OFF");
            printf("Volume: %.2f\n", volume);
        }

        // Auto layout example
        tinygui::beginLayout(400, menuHeight + 60, true, 10);
        tinygui::labelLayout("Quick Actions:", 2.0f, tinygui::COLOR_CYAN);
        if (tinygui::buttonLayout(150, 30, "Quick Save")) {
            printf("Quick save clicked!\n");
        }
        if (tinygui::buttonLayout(150, 30, "Load Defaults")) {
            volume = 0.5f;
            notifications = true;
        }

        tinygui::endFrame();
    }

    return 0;
}
```

---

## 🌟 **Why Choose NovaGUI?**

- ✅ **Beginner Friendly** - Perfect for C++ beginners
- ✅ **Single Header** - No complex build systems
- ✅ **Professional Look** - Modern UI with menus and themes
- ✅ **Full Featured** - All essential widgets included
- ✅ **Cross Platform** - Windows, macOS, Linux
- ✅ **Active Development** - Constantly improving
- ✅ **Zero Dependencies** - Just GLFW and OpenGL

---

## 📞 **Support**

Perfect for beginners learning C++ GUI programming! 🚀

**Happy Coding!** 💻✨