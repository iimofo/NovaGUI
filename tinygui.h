#pragma once
#define GL_SILENCE_DEPRECATION

// Platform OpenGL headers (fixed-function pipeline)
#if defined(_WIN32)
  #include <windows.h>
  #include <GL/gl.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>

#define STB_EASY_FONT_STATIC
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

// Include STB Image for loading image files
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <map>
#include <string>

namespace tinygui {

static constexpr int TINYGUI_MAX_TEXT = 256;
static constexpr float TINYGUI_LABEL_SCALE = 2.0f; // default label scale

// =============== Color System ===============
struct Color {
    float r, g, b, a;
    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    void apply() const { glColor4f(r, g, b, a); }
};

// Predefined colors
static const Color COLOR_WHITE(1.0f, 1.0f, 1.0f, 1.0f);
static const Color COLOR_BLACK(0.0f, 0.0f, 0.0f, 1.0f);
static const Color COLOR_GRAY(0.5f, 0.5f, 0.5f, 1.0f);
static const Color COLOR_LIGHT_GRAY(0.8f, 0.8f, 0.8f, 1.0f);
static const Color COLOR_DARK_GRAY(0.3f, 0.3f, 0.3f, 1.0f);
static const Color COLOR_RED(1.0f, 0.0f, 0.0f, 1.0f);
static const Color COLOR_GREEN(0.0f, 1.0f, 0.0f, 1.0f);
static const Color COLOR_BLUE(0.0f, 0.0f, 1.0f, 1.0f);
static const Color COLOR_YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
static const Color COLOR_ORANGE(1.0f, 0.5f, 0.0f, 1.0f);
static const Color COLOR_PURPLE(0.5f, 0.0f, 1.0f, 1.0f);
static const Color COLOR_CYAN(0.0f, 1.0f, 1.0f, 1.0f);

// UI Theme colors
static const Color THEME_BG(0.15f, 0.15f, 0.15f, 1.0f);           // Background
static const Color THEME_BUTTON(0.3f, 0.3f, 0.3f, 1.0f);          // Button normal
static const Color THEME_BUTTON_HOVER(0.7f, 0.3f, 0.3f, 1.0f);    // Button hovered
static const Color THEME_BUTTON_ACTIVE(0.5f, 0.2f, 0.2f, 1.0f);   // Button pressed
static const Color THEME_INPUT(0.3f, 0.3f, 0.3f, 1.0f);           // Input normal
static const Color THEME_INPUT_ACTIVE(0.6f, 0.3f, 0.3f, 1.0f);    // Input active
static const Color THEME_SELECTION(0.25f, 0.45f, 0.85f, 1.0f);    // Text selection
static const Color THEME_TEXT(1.0f, 1.0f, 1.0f, 1.0f);            // Normal text
static const Color THEME_TEXT_DIM(0.7f, 0.7f, 0.7f, 1.0f);        // Disabled/hint text

// Input field state
struct InputState {
    char text[TINYGUI_MAX_TEXT];
    int caret;
    int selAnchor;
    bool selecting;
    double blinkStart;
    float scrollOffset; // Horizontal scroll offset for long text
    
    InputState() {
        text[0] = 0;
        caret = selAnchor = 0;
        selecting = false;
        blinkStart = 0.0;
        scrollOffset = 0.0f;
    }
};

// Image data structure
struct ImageData {
    unsigned int textureID;
    int width, height;
    bool loaded;
    
    ImageData() : textureID(0), width(0), height(0), loaded(false) {}
};

// Multi-line text area state
struct TextAreaState {
    char text[TINYGUI_MAX_TEXT * 4]; // Larger buffer for multi-line
    int caret;
    int selAnchor;
    bool selecting;
    double blinkStart;
    float scrollY;
    
    TextAreaState() {
        text[0] = 0;
        caret = selAnchor = 0;
        selecting = false;
        blinkStart = 0.0;
        scrollY = 0.0f;
    }
};

// Modal dialog state
struct ModalState {
    bool visible;
    char title[256];
    char message[512];
    int result; // 0=none, 1=ok, 2=cancel, 3=yes, 4=no
    int type;   // 0=alert, 1=confirm, 2=yesno
    
    ModalState() {
        visible = false;
        title[0] = message[0] = 0;
        result = type = 0;
    }
};

struct Context {
    GLFWwindow* window;
    float mouseX, mouseY;
    bool mouseDown;
    bool mousePressed;
    
    // Active input field
    InputState* activeInput;
    
    // Layout state
    float layoutX, layoutY;  // Current layout position
    float layoutSpacing;     // Space between widgets
    bool layoutVertical;     // Layout direction
    
    // Menu system state
    bool menuBarVisible;
    int activeMenu;          // -1 = none, 0+ = menu index
    int hoveredMenu;         // -1 = none, 0+ = menu index
    float menuBarHeight;
    int pendingMenuResult;   // Store menu result for automatic handling
    
    // Image system
    std::map<std::string, ImageData> imageCache;
    
    // Modal dialog system
    ModalState modal;
};

static Context ctx;

// ============== Helpers for text editing ==============
inline int textLen(InputState* input) {
    if (!input) return 0;
    return (int)std::strlen(input->text);
}

inline int clampIndex(int i, InputState* input) {
    int len = textLen(input);
    return std::max(0, std::min(i, len));
}

inline bool hasSelection(InputState* input) {
    if (!input) return false;
    return input->caret != input->selAnchor;
}

inline void resetBlink(InputState* input) {
    if (!input) return;
    input->blinkStart = glfwGetTime();
}

inline void setCaret(InputState* input, int pos, bool keepSelection) {
    if (!input) return;
    pos = clampIndex(pos, input);
    input->caret = pos;
    if (!keepSelection) input->selAnchor = pos;
    resetBlink(input);
}

inline void deleteSelectionRange(InputState* input) {
    if (!input || !hasSelection(input)) return;
    int a = std::min(input->selAnchor, input->caret);
    int b = std::max(input->selAnchor, input->caret);
    int len = textLen(input);
    std::memmove(input->text + a, input->text + b, (size_t)(len - b + 1)); // include null terminator
    input->caret = input->selAnchor = a;
    resetBlink(input);
}

// Compute per-character widths and cumulative positions
// Returns length actually processed.
inline int computeCharWidths(const char* text, float scale,
                            float* charWidths, float* cumWidths, int maxChars,
                            float* outTotalWidth) {
    int len = (int)std::strlen(text);
    if (len > maxChars) len = maxChars;

    float totalWidth = 0.0f;
    for (int i = 0; i < len; ++i) {
        // Get character width from stb_easy_font character info
        unsigned char c = text[i];
        if (c >= 32 && c < 128) {
            // Get advance width (bottom 4 bits) and add spacing
            float charWidth = (stb_easy_font_charinfo[c - 32].advance & 15) * scale;
            charWidth += stb_easy_font_spacing_val * scale;
            charWidths[i] = charWidth;
        } else {
            charWidths[i] = 0.0f; // Invalid character
        }
        cumWidths[i] = totalWidth;
        totalWidth += charWidths[i];
    }
    
    if (outTotalWidth) *outTotalWidth = totalWidth;
    return len;
}

// ==================== Initialization ====================
inline bool init(int w, int h, const char* title) {
    if (!glfwInit()) return false;
    ctx.window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!ctx.window) return false;
    
    // Set minimum window size to 800x600
    glfwSetWindowSizeLimits(ctx.window, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    
    glfwMakeContextCurrent(ctx.window);

    ctx.mouseX = ctx.mouseY = 0;
    ctx.mouseDown = ctx.mousePressed = false;
    ctx.activeInput = nullptr;
    
    // Initialize layout
    ctx.layoutX = ctx.layoutY = 0;
    ctx.layoutSpacing = 8.0f;
    ctx.layoutVertical = true;
    
    // Initialize menu system
    ctx.menuBarVisible = true;
    ctx.activeMenu = -1;
    ctx.hoveredMenu = -1;
    ctx.menuBarHeight = 25.0f;
    ctx.pendingMenuResult = -1;

    return true;
}

inline bool windowShouldClose() { return glfwWindowShouldClose(ctx.window); }

// ==================== Event Handling ====================
inline void pollEvents() {
    ctx.mousePressed = false;
    glfwPollEvents();

    double x, y;
    glfwGetCursorPos(ctx.window, &x, &y);
    ctx.mouseX = (float)x; // Window coords: origin top-left, Y down (matches our ortho)
    ctx.mouseY = (float)y;

    bool down = glfwGetMouseButton(ctx.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    ctx.mousePressed = down && !ctx.mouseDown;
    ctx.mouseDown = down;
}

// ==================== Frame Rendering ====================
inline void beginFrame() {
    // Use framebuffer size for viewport (pixels)
    int fbW, fbH;
    glfwGetFramebufferSize(ctx.window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use window size for 2D coordinates (logical units)
    int w, h;
    glfwGetWindowSize(ctx.window, &w, &h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Top-left origin, Y down
    glOrtho(0, w, h, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// endFrame will be defined after menu functions
inline void endFrame();

// Get the menu result from the last frame
inline int getMenuResult() {
    int result = ctx.pendingMenuResult;
    ctx.pendingMenuResult = -1; // Clear after reading
    return result;
}

// ==================== Utility Functions ====================
inline void drawRect(float x, float y, float w, float h, const Color& color = COLOR_WHITE) {
    color.apply();
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

inline void drawRectOutline(float x, float y, float w, float h, const Color& color = COLOR_WHITE, float thickness = 1.0f) {
    color.apply();
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
    glLineWidth(1.0f);
}

inline void drawLine(float x1, float y1, float x2, float y2, const Color& color = COLOR_WHITE, float thickness = 1.0f) {
    color.apply();
    glLineWidth(thickness);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

inline float measureTextWidth(const char* text, float scale = TINYGUI_LABEL_SCALE) {
    return stb_easy_font_width((char*)text) * scale;
}

inline float measureTextHeight(const char* text, float scale = TINYGUI_LABEL_SCALE) {
    return stb_easy_font_height((char*)text) * scale;
}

inline bool pointInRect(float px, float py, float x, float y, float w, float h) {
    return px >= x && px <= x + w && py >= y && py <= y + h;
}

// Forward declarations for functions that need to be defined later
inline void drawModalDialog();
inline void image(float x, float y, float w, float h, const char* name);
inline int tabBar(float x, float y, float w, float h, const char** tabNames, int tabCount, int& activeTab);

// ==================== Modal Dialog System (definitions moved later) ====================
// Modal dialog function declarations (implementations after widgets)
inline void alert(const char* title, const char* message);
inline bool confirm(const char* title, const char* message);
inline bool isModalVisible();

// ==================== Image System ====================
// Image loading function declarations (implementations after widgets)
inline bool loadImageFromData(const char* name, unsigned char* pixels, int width, int height) {
    ImageData& img = ctx.imageCache[name];
    if (img.loaded) return true;
    
    glGenTextures(1, &img.textureID);
    glBindTexture(GL_TEXTURE_2D, img.textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    img.width = width; img.height = height; img.loaded = true;
    return true;
}

inline bool createTestImage(const char* name, int size = 64) {
    unsigned char* pixels = new unsigned char[size * size * 4];
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int idx = (y * size + x) * 4;
            bool checker = ((x / 8) + (y / 8)) % 2 == 0;
            pixels[idx + 0] = checker ? 255 : 100;
            pixels[idx + 1] = checker ? 100 : 255;
            pixels[idx + 2] = 100;
            pixels[idx + 3] = 255;
        }
    }
    bool result = loadImageFromData(name, pixels, size, size);
    delete[] pixels;
    return result;
}

// Load image from file (PNG, JPG, BMP, TGA, etc.)
inline bool loadImageFromFile(const char* name, const char* filepath) {
    int width, height, channels;
    unsigned char* pixels = stbi_load(filepath, &width, &height, &channels, 4); // Force RGBA
    
    if (!pixels) {
        printf("Failed to load image: %s\n", filepath);
        return false;
    }
    
    bool result = loadImageFromData(name, pixels, width, height);
    stbi_image_free(pixels);
    
    if (result) {
        printf("Successfully loaded image: %s (%dx%d)\n", filepath, width, height);
    }
    
    return result;
}

// ==================== GUI Widgets ====================
inline void label(float x, float y, const char* text, float scale = TINYGUI_LABEL_SCALE, const Color& color = THEME_TEXT) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);

    // Each char uses 64 bytes (4 verts * 16 bytes); allow generous buffer
    char buffer[16384];
    int num_quads = stb_easy_font_print(0, 0, (char*)text, NULL, buffer, sizeof(buffer));

    color.apply();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

// Helper function to calculate minimum button size for text + padding
inline void getMinButtonSize(const char* text, float& minW, float& minH, float padding = 8.0f) {
    const float s = TINYGUI_LABEL_SCALE;
    int tw = stb_easy_font_width((char*)text);
    int th = stb_easy_font_height((char*)text);
    minW = tw * s + 2 * padding;
    minH = th * s + 2 * padding;
}

inline bool button(float x, float y, float w, float h, const char* text) {
    // Hit-test in window coords (no Y flip)
    bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, x, y, w, h);
    bool pressed = hovered && ctx.mouseDown;

    // Choose button color based on state
    const Color& buttonColor = pressed ? THEME_BUTTON_ACTIVE : 
                              (hovered ? THEME_BUTTON_HOVER : THEME_BUTTON);
    
    drawRect(x, y, w, h, buttonColor);

    // Center text using stb metrics at the same scale as label
    const float s = TINYGUI_LABEL_SCALE;
    int tw = stb_easy_font_width((char*)text);
    int th = stb_easy_font_height((char*)text);
    float textW = tw * s;
    float textH = th * s;
    
    // Always center text perfectly within the button bounds
    float tx = x + (w - textW) * 0.5f;
    float ty = y + (h - textH) * 0.5f;
    
    label(tx, ty, text, s);

    return hovered && ctx.mousePressed;
}

// Auto-sizing button that expands to fit text + padding
inline bool autoButton(float x, float y, const char* text, float padding = 8.0f) {
    float w, h;
    getMinButtonSize(text, w, h, padding);
    return button(x, y, w, h, text);
}

// ==================== Vertical Sidebar Tabs ====================
// Vertical sidebar tab system for left-side navigation

// Single vertical tab button
inline bool verticalTab(float x, float y, float w, float h, const char* text, bool isActive, bool showIcon = false) {
    bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, x, y, w, h);
    bool pressed = hovered && ctx.mouseDown;
    
    // Choose colors based on state
    Color bgColor;
    if (isActive) {
        bgColor = THEME_BUTTON_ACTIVE;
    } else if (hovered) {
        bgColor = THEME_BUTTON_HOVER;
    } else {
        bgColor = Color(0.2f, 0.2f, 0.2f, 1.0f); // Darker sidebar background
    }
    
    // Draw tab background
    drawRect(x, y, w, h, bgColor);
    
    // Draw active indicator (left border)
    if (isActive) {
        drawRect(x, y, 3.0f, h, COLOR_CYAN); // Accent color for active tab
    }
    
    // Draw text (rotated or horizontal depending on preference)
    const float s = 1.8f; // Slightly smaller text for sidebar
    float textW = measureTextWidth(text, s);
    float textH = measureTextHeight(text, s);
    
    // Center text in the tab
    float tx = x + (w - textW) * 0.5f;
    float ty = y + (h - textH) * 0.5f;
    
    Color textColor = isActive ? THEME_TEXT : (hovered ? THEME_TEXT : THEME_TEXT_DIM);
    label(tx, ty, text, s, textColor);
    
    // Draw separator line at bottom
    drawLine(x + 5, y + h, x + w - 5, y + h, THEME_TEXT_DIM, 0.5f);
    
    return hovered && ctx.mousePressed;
}

// Vertical sidebar tab bar
inline int verticalTabBar(float x, float y, float w, const char** tabNames, int tabCount, int& activeTab, float tabHeight = 50.0f) {
    if (tabCount <= 0) return -1;
    
    // Draw sidebar background
    int windowW, windowH;
    glfwGetWindowSize(ctx.window, &windowW, &windowH);
    float sidebarHeight = windowH - y;
    drawRect(x, y, w, sidebarHeight, Color(0.15f, 0.15f, 0.15f, 1.0f));
    
    int clickedTab = -1;
    
    for (int i = 0; i < tabCount; i++) {
        float tabY = y + i * tabHeight;
        bool isActive = (i == activeTab);
        
        if (verticalTab(x, tabY, w, tabHeight, tabNames[i], isActive)) {
            activeTab = i;
            clickedTab = i;
        }
    }
    
    // Draw right border of sidebar
    drawLine(x + w, y, x + w, y + sidebarHeight, THEME_TEXT_DIM, 1.0f);
    
    return clickedTab;
}

// Helper function to get sidebar width
inline float getSidebarWidth() {
    return 120.0f; // Standard sidebar width
}

// ==================== Input Box (with caret & selection) ====================
inline bool input(float x, float y, float w, float h, InputState& inputState, const char* hint = "") {
    bool inside = pointInRect(ctx.mouseX, ctx.mouseY, x, y, w, h);
    bool isActive = (ctx.activeInput == &inputState);

    bool shiftDown = glfwGetKey(ctx.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                     glfwGetKey(ctx.window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    const float s = TINYGUI_LABEL_SCALE;
    const float padX = 5.0f, padY = 5.0f;
    const float lineH = 8.0f * s;
    const float textAreaW = w - 2 * padX;

    // Precompute character widths for current text
    float charWidths[TINYGUI_MAX_TEXT]{};
    float cumWidths[TINYGUI_MAX_TEXT]{};
    float totalTextWidth = 0.0f;
    int len = computeCharWidths(inputState.text, s, charWidths, cumWidths, TINYGUI_MAX_TEXT - 1, &totalTextWidth);

    // Calculate caret position and adjust scroll offset
    auto caretXAt = [&](int idx) -> float {
        if (idx <= 0) return 0.0f;
        if (idx >= len) return totalTextWidth;
        return cumWidths[idx];
    };

    // Ensure caret is visible by adjusting scroll offset
    if (isActive) {
        float caretPos = caretXAt(inputState.caret);
        float visibleStart = inputState.scrollOffset;
        float visibleEnd = inputState.scrollOffset + textAreaW;
        
        // Scroll right if caret is beyond visible area
        if (caretPos > visibleEnd - 10) {
            inputState.scrollOffset = caretPos - textAreaW + 10;
        }
        // Scroll left if caret is before visible area
        else if (caretPos < visibleStart + 10) {
            inputState.scrollOffset = std::max(0.0f, caretPos - 10);
        }
        
        // Clamp scroll offset
        inputState.scrollOffset = std::max(0.0f, std::min(inputState.scrollOffset, std::max(0.0f, totalTextWidth - textAreaW)));
    }

    auto indexFromX = [&](float mouseX) -> int {
        float lx = (mouseX - (x + padX)) + inputState.scrollOffset;
        if (lx <= 0.0f) return 0;
        if (lx >= totalTextWidth) return len;
        
        // Find the character position closest to the mouse
        for (int i = 0; i < len; ++i) {
            float charStart = cumWidths[i];
            float charEnd = charStart + charWidths[i];
            float charMid = (charStart + charEnd) * 0.5f;
            if (lx < charMid) return i;
        }
        return len;
    };

    if (ctx.mousePressed && inside) {
        ctx.activeInput = &inputState;
        isActive = true;
        int newCaret = indexFromX(ctx.mouseX);
        if (!shiftDown) inputState.selAnchor = newCaret;
        inputState.caret = newCaret;
        inputState.selecting = true;
        glfwFocusWindow(ctx.window);
        resetBlink(&inputState);
    } else if (ctx.mousePressed && !inside && isActive) {
        ctx.activeInput = nullptr;
        inputState.selecting = false;
    } else if (!ctx.mouseDown) {
        inputState.selecting = false;
    }

    // Drag selection
    if (isActive && inputState.selecting) {
        inputState.caret = indexFromX(ctx.mouseX);
        resetBlink(&inputState);
    }

    // Draw box
    const Color& inputColor = isActive ? THEME_INPUT_ACTIVE : THEME_INPUT;
    drawRect(x, y, w, h, inputColor);
    
    // Enable scissor test for text clipping
    glEnable(GL_SCISSOR_TEST);
    int windowW, windowH;
    glfwGetWindowSize(ctx.window, &windowW, &windowH);
    glScissor((int)(x + padX), windowH - (int)(y + h - padY), (int)textAreaW, (int)(h - 2 * padY));

    // Draw selection (behind text)
    if (isActive && hasSelection(&inputState)) {
        int a = std::min(inputState.selAnchor, inputState.caret);
        int b = std::max(inputState.selAnchor, inputState.caret);
        float selX0 = x + padX + caretXAt(a) - inputState.scrollOffset;
        float selX1 = x + padX + caretXAt(b) - inputState.scrollOffset;

        drawRect(selX0, y + padY, selX1 - selX0, lineH, THEME_SELECTION);
    }

    // Draw text or hint
    if (inputState.text[0] != 0) {
        label(x + padX - inputState.scrollOffset, y + padY, inputState.text, s, THEME_TEXT);
    } else if (hint[0] != 0 && !isActive) {
        label(x + padX, y + padY, hint, s, THEME_TEXT_DIM);
    }

    // Draw caret (blinking)
    if (isActive) {
        double t = glfwGetTime();
        bool showCaret = fmod(t - inputState.blinkStart, 1.0) < 0.5;
        if (showCaret) {
            float cx = x + padX + caretXAt(inputState.caret) - inputState.scrollOffset;
            drawLine(cx, y + padY, cx, y + padY + lineH, THEME_TEXT);
        }
    }
    
    // Disable scissor test
    glDisable(GL_SCISSOR_TEST);
    
    // Draw input border
    drawRectOutline(x, y, w, h, isActive ? THEME_TEXT : THEME_TEXT_DIM, 1.0f);

    return isActive;
}

// ==================== Additional Widgets ====================

// Checkbox widget
inline bool checkbox(float x, float y, float size, const char* text, bool& checked) {
    bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, x, y, size, size);
    
    if (hovered && ctx.mousePressed) {
        checked = !checked;
    }
    
    // Draw checkbox box
    const Color& boxColor = hovered ? THEME_BUTTON_HOVER : THEME_INPUT;
    drawRect(x, y, size, size, boxColor);
    drawRectOutline(x, y, size, size, THEME_TEXT, 1.0f);
    
    // Draw checkmark if checked
    if (checked) {
        float margin = size * 0.2f;
        drawLine(x + margin, y + size * 0.5f, x + size * 0.4f, y + size - margin, THEME_TEXT, 2.0f);
        drawLine(x + size * 0.4f, y + size - margin, x + size - margin, y + margin, THEME_TEXT, 2.0f);
    }
    
    // Draw label
    if (text && text[0]) {
        label(x + size + 8, y + (size - measureTextHeight(text)) * 0.5f, text, TINYGUI_LABEL_SCALE, THEME_TEXT);
    }
    
    return hovered && ctx.mousePressed;
}

// Slider widget
inline bool slider(float x, float y, float w, float h, float& value, float minValue = 0.0f, float maxValue = 1.0f) {
    bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, x, y, w, h);
    bool dragging = hovered && ctx.mouseDown;
    
    if (dragging) {
        float relativeX = ctx.mouseX - x;
        float t = relativeX / w;
        t = std::max(0.0f, std::min(1.0f, t));
        value = minValue + t * (maxValue - minValue);
    }
    
    // Ensure proper OpenGL state for drawing
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw track
    float trackY = y + h * 0.4f;
    float trackH = h * 0.2f;
    drawRect(x, trackY, w, trackH, THEME_INPUT);
    
    // Draw track outline to make it more visible
    drawRectOutline(x, trackY, w, trackH, THEME_TEXT_DIM, 1.0f);
    
    // Draw handle
    float t = (value - minValue) / (maxValue - minValue);
    float handleX = x + t * w;
    float handleW = 8.0f;
    float handleH = h;
    
    const Color& handleColor = dragging ? THEME_BUTTON_ACTIVE : 
                              (hovered ? THEME_BUTTON_HOVER : THEME_BUTTON);
    drawRect(handleX - handleW * 0.5f, y, handleW, handleH, handleColor);
    
    return dragging;
}

// Progress bar widget
inline void progressBar(float x, float y, float w, float h, float progress, const Color& fillColor = COLOR_GREEN) {
    progress = std::max(0.0f, std::min(1.0f, progress));
    
    // Ensure proper OpenGL state for drawing
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw background
    drawRect(x, y, w, h, THEME_INPUT);
    
    // Draw progress fill
    if (progress > 0.0f) {
        drawRect(x, y, w * progress, h, fillColor);
    }
    
    // Draw border
    drawRectOutline(x, y, w, h, THEME_TEXT, 1.0f);
}

// ==================== List & Dropdown Widgets ====================
// List box widget
inline int listBox(float x, float y, float w, float h, const char** items, int itemCount, int& selectedIndex) {
    if (itemCount <= 0) return -1;
    
    // Draw list background
    drawRect(x, y, w, h, THEME_INPUT);
    drawRectOutline(x, y, w, h, THEME_TEXT, 1.0f);
    
    float itemHeight = 25.0f;
    int visibleItems = (int)(h / itemHeight);
    int clickedItem = -1;
    
    // Enable clipping
    glEnable(GL_SCISSOR_TEST);
    int windowW, windowH;
    glfwGetWindowSize(ctx.window, &windowW, &windowH);
    glScissor((int)x, windowH - (int)(y + h), (int)w, (int)h);
    
    for (int i = 0; i < itemCount && i < visibleItems; i++) {
        float itemY = y + i * itemHeight;
        bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, x, itemY, w, itemHeight);
        bool isSelected = (i == selectedIndex);
        
        // Item background
        if (isSelected) {
            drawRect(x, itemY, w, itemHeight, THEME_SELECTION);
        } else if (hovered) {
            drawRect(x, itemY, w, itemHeight, THEME_BUTTON_HOVER);
        }
        
        // Item text
        label(x + 5, itemY + 3, items[i], 1.8f, THEME_TEXT);
        
        // Handle click
        if (hovered && ctx.mousePressed) {
            selectedIndex = i;
            clickedItem = i;
        }
    }
    
    glDisable(GL_SCISSOR_TEST);
    return clickedItem;
}

// Dropdown widget
inline int dropdown(float x, float y, float w, float h, const char** items, int itemCount, int& selectedIndex, bool& isOpen) {
    if (itemCount <= 0) return -1;
    
    // Main dropdown button
    bool buttonHovered = pointInRect(ctx.mouseX, ctx.mouseY, x, y, w, h);
    const Color& buttonColor = buttonHovered ? THEME_BUTTON_HOVER : THEME_INPUT;
    drawRect(x, y, w, h, buttonColor);
    drawRectOutline(x, y, w, h, THEME_TEXT, 1.0f);
    
    // Selected item text
    if (selectedIndex >= 0 && selectedIndex < itemCount) {
        label(x + 5, y + 3, items[selectedIndex], 1.8f, THEME_TEXT);
    } else {
        label(x + 5, y + 3, "Select...", 1.8f, THEME_TEXT_DIM);
    }
    
    // Dropdown arrow
    float arrowSize = 8.0f;
    float arrowX = x + w - arrowSize - 5;
    float arrowY = y + h * 0.5f;
    label(arrowX, arrowY - arrowSize * 0.5f, "v", 1.5f, THEME_TEXT);
    
    // Toggle dropdown on click
    if (buttonHovered && ctx.mousePressed) {
        isOpen = !isOpen;
    }
    
    int clickedItem = -1;
    
    // Draw dropdown list if open
    if (isOpen) {
        float listY = y + h;
        float listH = std::min((float)itemCount * 25.0f, 150.0f);
        
        // Draw list background
        drawRect(x, listY, w, listH, THEME_INPUT);
        drawRectOutline(x, listY, w, listH, THEME_TEXT, 1.0f);
        
        // Enable clipping for list
        glEnable(GL_SCISSOR_TEST);
        int windowW, windowH;
        glfwGetWindowSize(ctx.window, &windowW, &windowH);
        glScissor((int)x, windowH - (int)(listY + listH), (int)w, (int)listH);
        
        for (int i = 0; i < itemCount; i++) {
            float itemY = listY + i * 25.0f;
            bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, x, itemY, w, 25.0f);
            bool isSelected = (i == selectedIndex);
            
            // Item background
            if (isSelected) {
                drawRect(x, itemY, w, 25.0f, THEME_SELECTION);
            } else if (hovered) {
                drawRect(x, itemY, w, 25.0f, THEME_BUTTON_HOVER);
            }
            
            // Item text
            label(x + 5, itemY + 3, items[i], 1.8f, THEME_TEXT);
            
            // Handle click
            if (hovered && ctx.mousePressed) {
                selectedIndex = i;
                clickedItem = i;
                isOpen = false; // Close dropdown
            }
        }
        
        glDisable(GL_SCISSOR_TEST);
        
        // Close dropdown if clicked outside
        if (ctx.mousePressed && !pointInRect(ctx.mouseX, ctx.mouseY, x, y, w, h + listH)) {
            isOpen = false;
        }
    }
    
    return clickedItem;
}

// Simple dropdown without state management
inline int simpleDropdown(float x, float y, float w, float h, const char** items, int itemCount, int& selectedIndex) {
    static bool isOpen = false;
    return dropdown(x, y, w, h, items, itemCount, selectedIndex, isOpen);
}

// ==================== Layout Management ====================

// Set layout starting position and direction
inline void beginLayout(float x, float y, bool vertical = true, float spacing = 8.0f) {
    ctx.layoutX = x;
    ctx.layoutY = y;
    ctx.layoutVertical = vertical;
    ctx.layoutSpacing = spacing;
}

// Get current layout position
inline void getLayoutPos(float& x, float& y) {
    x = ctx.layoutX;
    y = ctx.layoutY;
}

// Advance layout position
inline void advanceLayout(float w, float h) {
    if (ctx.layoutVertical) {
        ctx.layoutY += h + ctx.layoutSpacing;
    } else {
        ctx.layoutX += w + ctx.layoutSpacing;
    }
}

// Layout-aware button
inline bool buttonLayout(float w, float h, const char* text) {
    bool result = button(ctx.layoutX, ctx.layoutY, w, h, text);
    advanceLayout(w, h);
    return result;
}

// Layout-aware label
inline void labelLayout(const char* text, float scale = TINYGUI_LABEL_SCALE, const Color& color = THEME_TEXT) {
    label(ctx.layoutX, ctx.layoutY, text, scale, color);
    float h = measureTextHeight(text, scale);
    advanceLayout(0, h);
}

// Layout-aware input
inline bool inputLayout(float w, float h, InputState& inputState, const char* hint = "") {
    bool result = input(ctx.layoutX, ctx.layoutY, w, h, inputState, hint);
    advanceLayout(w, h);
    return result;
}

// Layout-aware checkbox
inline bool checkboxLayout(float size, const char* text, bool& checked) {
    bool result = checkbox(ctx.layoutX, ctx.layoutY, size, text, checked);
    advanceLayout(size + (text ? measureTextWidth(text) + 8 : 0), size);
    return result;
}

// Layout-aware slider
inline bool sliderLayout(float w, float h, float& value, float minValue = 0.0f, float maxValue = 1.0f) {
    bool result = slider(ctx.layoutX, ctx.layoutY, w, h, value, minValue, maxValue);
    advanceLayout(w, h);
    return result;
}

// Add spacing to layout
inline void layoutSpacing(float space = -1.0f) {
    if (space < 0) space = ctx.layoutSpacing;
    if (ctx.layoutVertical) {
        ctx.layoutY += space;
    } else {
        ctx.layoutX += space;
    }
}

// ==================== Menu System ====================

// Menu item structure
struct MenuItem {
    const char* text;
    bool enabled;
    bool separator;
    
    MenuItem(const char* t = "", bool e = true, bool s = false) 
        : text(t), enabled(e), separator(s) {}
};

// Menu positions storage
static float menuPositions[10]; // Support up to 10 menus

// Draw menu bar
inline bool menuBar(const char** menuTitles, int menuCount) {
    if (!ctx.menuBarVisible) return false;
    
    int windowW, windowH;
    glfwGetWindowSize(ctx.window, &windowW, &windowH);
    
    // Draw menu bar background
    drawRect(0, 0, (float)windowW, ctx.menuBarHeight, THEME_BUTTON);
    drawLine(0, ctx.menuBarHeight, (float)windowW, ctx.menuBarHeight, THEME_TEXT, 1.0f);
    
    float currentX = 10.0f;
    bool menuClicked = false;
    
    for (int i = 0; i < menuCount && i < 10; i++) {
        float textW = measureTextWidth(menuTitles[i], 1.8f);
        float menuW = textW + 20.0f;
        
        // Store menu position for dropdown alignment
        menuPositions[i] = currentX;
        
        bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, currentX, 0, menuW, ctx.menuBarHeight);
        bool isActive = (ctx.activeMenu == i);
        
        // Draw menu title background if hovered or active
        if (hovered || isActive) {
            drawRect(currentX, 0, menuW, ctx.menuBarHeight, 
                    isActive ? THEME_BUTTON_ACTIVE : THEME_BUTTON_HOVER);
        }
        
        // Draw menu title text
        label(currentX + 10, 4, menuTitles[i], 1.8f, THEME_TEXT);
        
        // Handle menu interaction
        if (hovered) {
            ctx.hoveredMenu = i;
            if (ctx.mousePressed) {
                ctx.activeMenu = (ctx.activeMenu == i) ? -1 : i;
                menuClicked = true;
            }
        }
        
        currentX += menuW + 5.0f;
    }
    
    // Close menu if clicked outside
    if (ctx.mousePressed && ctx.mouseY > ctx.menuBarHeight) {
        ctx.activeMenu = -1;
    }
    
    return menuClicked;
}

// Draw dropdown menu
inline int dropdownMenu(float x, float y, MenuItem* items, int itemCount) {
    if (ctx.activeMenu == -1) return -1;
    
    float menuW = 150.0f;
    float itemH = 25.0f;
    float menuH = itemCount * itemH;
    
    // Draw menu background
    drawRect(x, y, menuW, menuH, THEME_INPUT);
    drawRectOutline(x, y, menuW, menuH, THEME_TEXT, 1.0f);
    
    int clickedItem = -1;
    
    for (int i = 0; i < itemCount; i++) {
        float itemY = y + i * itemH;
        
        if (items[i].separator) {
            // Draw separator line
            drawLine(x + 5, itemY + itemH * 0.5f, x + menuW - 5, itemY + itemH * 0.5f, THEME_TEXT_DIM, 1.0f);
        } else {
            bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, x, itemY, menuW, itemH);
            
            // Draw item background if hovered
            if (hovered && items[i].enabled) {
                drawRect(x, itemY, menuW, itemH, THEME_BUTTON_HOVER);
            }
            
            // Draw item text
            Color textColor = items[i].enabled ? THEME_TEXT : THEME_TEXT_DIM;
            label(x + 10, itemY + 3, items[i].text, 1.8f, textColor);
            
            // Handle click
            if (hovered && ctx.mousePressed && items[i].enabled) {
                clickedItem = i;
                ctx.activeMenu = -1; // Close menu after selection
            }
        }
    }
    
    return clickedItem;
}

// Convenience function for common menu - just draws menu bar
inline void standardMenuBarOnly() {
    static const char* menuTitles[] = {"File", "Edit", "View", "Help"};
    menuBar(menuTitles, 4);
}

// Draw dropdown menus - call this AFTER all other GUI elements
inline int standardMenuDropdowns() {
    static MenuItem fileMenu[] = {
        MenuItem("New", true),
        MenuItem("Open", true),
        MenuItem("Save", true),
        MenuItem("", true, true), // separator
        MenuItem("Exit", true)
    };
    static MenuItem editMenu[] = {
        MenuItem("Undo", false),
        MenuItem("Redo", false),
        MenuItem("", true, true), // separator
        MenuItem("Cut", true),
        MenuItem("Copy", true),
        MenuItem("Paste", true)
    };
    static MenuItem viewMenu[] = {
        MenuItem("Zoom In", true),
        MenuItem("Zoom Out", true),
        MenuItem("Reset Zoom", true)
    };
    static MenuItem helpMenu[] = {
        MenuItem("About", true),
        MenuItem("Documentation", true)
    };
    
    int result = -1;
    
    switch (ctx.activeMenu) {
        case 0: // File menu
            result = dropdownMenu(menuPositions[0], ctx.menuBarHeight + 1, fileMenu, 5);
            if (result >= 0) result += 100; // File menu items: 100-104
            break;
        case 1: // Edit menu
            result = dropdownMenu(menuPositions[1], ctx.menuBarHeight + 1, editMenu, 6);
            if (result >= 0) result += 200; // Edit menu items: 200-205
            break;
        case 2: // View menu
            result = dropdownMenu(menuPositions[2], ctx.menuBarHeight + 1, viewMenu, 3);
            if (result >= 0) result += 300; // View menu items: 300-302
            break;
        case 3: // Help menu
            result = dropdownMenu(menuPositions[3], ctx.menuBarHeight + 1, helpMenu, 2);
            if (result >= 0) result += 400; // Help menu items: 400-401
            break;
    }
    
    return result;
}

// Convenience function for common menu (backward compatibility)
inline int standardMenuBar() {
    standardMenuBarOnly();
    return standardMenuDropdowns();
}

// Get menu bar height (for adjusting content position)
inline float getMenuBarHeight() {
    return ctx.menuBarVisible ? ctx.menuBarHeight : 0.0f;
}

// ==================== Easy Menu System ====================
// Simple function - just call this once at the beginning of your frame!
inline int easyMenuBar() {
    standardMenuBarOnly();
    return getMenuResult(); // Returns result from previous frame
}

// ==================== Function Implementations ====================
// Modal Dialog implementations
inline void alert(const char* title, const char* message) {
    strncpy(ctx.modal.title, title, sizeof(ctx.modal.title) - 1);
    strncpy(ctx.modal.message, message, sizeof(ctx.modal.message) - 1);
    ctx.modal.type = 0; // Alert
    ctx.modal.visible = true;
    ctx.modal.result = 0;
}

inline bool confirm(const char* title, const char* message) {
    if (!ctx.modal.visible) {
        strncpy(ctx.modal.title, title, sizeof(ctx.modal.title) - 1);
        strncpy(ctx.modal.message, message, sizeof(ctx.modal.message) - 1);
        ctx.modal.type = 1; // Confirm
        ctx.modal.visible = true;
        ctx.modal.result = 0;
    }
    return ctx.modal.result == 1;
}

inline bool isModalVisible() {
    return ctx.modal.visible;
}

inline void drawModalDialog() {
    if (!ctx.modal.visible) return;
    
    int windowW, windowH;
    glfwGetWindowSize(ctx.window, &windowW, &windowH);
    
    // Draw overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Color overlay(0.0f, 0.0f, 0.0f, 0.5f);
    drawRect(0, 0, (float)windowW, (float)windowH, overlay);
    
    // Dialog dimensions
    float dialogW = 400.0f;
    float dialogH = 200.0f;
    float dialogX = (windowW - dialogW) * 0.5f;
    float dialogY = (windowH - dialogH) * 0.5f;
    
    // Draw dialog background
    drawRect(dialogX, dialogY, dialogW, dialogH, THEME_INPUT);
    drawRectOutline(dialogX, dialogY, dialogW, dialogH, THEME_TEXT, 2.0f);
    
    // Title bar
    drawRect(dialogX, dialogY, dialogW, 30.0f, THEME_BUTTON);
    label(dialogX + 10, dialogY + 5, ctx.modal.title, 2.0f, THEME_TEXT);
    
    // Message
    label(dialogX + 20, dialogY + 50, ctx.modal.message, 2.0f, THEME_TEXT);
    
    // Buttons
    float buttonW = 80.0f;
    float buttonH = 30.0f;
    float buttonY = dialogY + dialogH - buttonH - 20.0f;
    
    if (ctx.modal.type == 0) { // Alert
        float buttonX = dialogX + (dialogW - buttonW) * 0.5f;
        if (button(buttonX, buttonY, buttonW, buttonH, "OK")) {
            ctx.modal.visible = false;
            ctx.modal.result = 1;
        }
    } else if (ctx.modal.type == 1) { // Confirm
        float okX = dialogX + dialogW * 0.3f - buttonW * 0.5f;
        float cancelX = dialogX + dialogW * 0.7f - buttonW * 0.5f;
        
        if (button(okX, buttonY, buttonW, buttonH, "OK")) {
            ctx.modal.visible = false;
            ctx.modal.result = 1;
        }
        if (button(cancelX, buttonY, buttonW, buttonH, "Cancel")) {
            ctx.modal.visible = false;
            ctx.modal.result = 2;
        }
    }
    
    if (glfwGetKey(ctx.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        ctx.modal.visible = false;
        ctx.modal.result = 2;
    }
    
    glDisable(GL_BLEND);
}

// Image system implementations
inline void image(float x, float y, float w, float h, const char* name) {
    auto it = ctx.imageCache.find(name);
    if (it == ctx.imageCache.end() || !it->second.loaded) {
        drawRect(x, y, w, h, COLOR_DARK_GRAY);
        drawRectOutline(x, y, w, h, COLOR_RED, 2.0f);
        label(x + 5, y + 5, "IMG?", 1.0f, COLOR_RED);
        return;
    }
    
    ImageData& img = it->second;
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, img.textureID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + w, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + w, y + h);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + h);
    glEnd();
    
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

// Tab system implementations
inline int tabBar(float x, float y, float w, float h, const char** tabNames, int tabCount, int& activeTab) {
    if (tabCount <= 0) return -1;
    
    float tabWidth = w / tabCount;
    int clickedTab = -1;
    
    for (int i = 0; i < tabCount; i++) {
        float tabX = x + i * tabWidth;
        bool isActive = (i == activeTab);
        bool hovered = pointInRect(ctx.mouseX, ctx.mouseY, tabX, y, tabWidth, h);
        
        Color tabColor = isActive ? THEME_BUTTON_ACTIVE : (hovered ? THEME_BUTTON_HOVER : THEME_BUTTON);
        drawRect(tabX, y, tabWidth, h, tabColor);
        
        if (isActive) {
            drawRectOutline(tabX, y, tabWidth, h, THEME_TEXT, 2.0f);
        } else {
            drawRectOutline(tabX, y, tabWidth, h, THEME_TEXT_DIM, 1.0f);
        }
        
        float textW = measureTextWidth(tabNames[i], 1.8f);
        float textX = tabX + (tabWidth - textW) * 0.5f;
        float textY = y + (h - measureTextHeight(tabNames[i], 1.8f)) * 0.5f;
        label(textX, textY, tabNames[i], 1.8f, THEME_TEXT);
        
        if (hovered && ctx.mousePressed) {
            activeTab = i;
            clickedTab = i;
        }
    }
    
    return clickedTab;
}

inline void beginTabContent(float x, float y, float w, float h) {
    drawRect(x, y, w, h, THEME_INPUT);
    drawRectOutline(x, y, w, h, THEME_TEXT, 1.0f);
    
    glEnable(GL_SCISSOR_TEST);
    int windowW, windowH;
    glfwGetWindowSize(ctx.window, &windowW, &windowH);
    glScissor((int)x, windowH - (int)(y + h), (int)w, (int)h);
}

inline void endTabContent() {
    glDisable(GL_SCISSOR_TEST);
}

// Define endFrame here after menu functions are available
inline void endFrame() { 
    // Automatically draw dropdown menus at the end of frame
    ctx.pendingMenuResult = standardMenuDropdowns();
    
    // Draw modal dialogs on top of everything
    drawModalDialog();
    
    glfwSwapBuffers(ctx.window); 
}

// ==================== Character and Key handling ====================
inline void addCharToInput(unsigned int codepoint) {
    if (!ctx.activeInput) return;
    if (codepoint < 32 || codepoint > 126) return; // printable ASCII only
    if (hasSelection(ctx.activeInput)) deleteSelectionRange(ctx.activeInput);

    int len = textLen(ctx.activeInput);
    if (len >= TINYGUI_MAX_TEXT - 1) return;

    int pos = clampIndex(ctx.activeInput->caret, ctx.activeInput);
    std::memmove(ctx.activeInput->text + pos + 1, ctx.activeInput->text + pos, (size_t)(len - pos + 1)); // includes null
    ctx.activeInput->text[pos] = (char)codepoint;
    ctx.activeInput->caret = ctx.activeInput->selAnchor = pos + 1;
    resetBlink(ctx.activeInput);
}

inline void onChar(unsigned int codepoint) { addCharToInput(codepoint); }

inline void handleKey(int key, int action, int mods) {
    if (!ctx.activeInput) return;
    if (!(action == GLFW_PRESS || action == GLFW_REPEAT)) return;

    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    int len = textLen(ctx.activeInput);

    auto moveTo = [&](int pos) {
        pos = clampIndex(pos, ctx.activeInput);
        if (!shift) ctx.activeInput->selAnchor = pos;
        ctx.activeInput->caret = pos;
        resetBlink(ctx.activeInput);
    };

    switch (key) {
        case GLFW_KEY_LEFT:
            if (hasSelection(ctx.activeInput) && !shift) moveTo(std::min(ctx.activeInput->selAnchor, ctx.activeInput->caret));
            else moveTo(ctx.activeInput->caret - 1);
            break;
        case GLFW_KEY_RIGHT:
            if (hasSelection(ctx.activeInput) && !shift) moveTo(std::max(ctx.activeInput->selAnchor, ctx.activeInput->caret));
            else moveTo(ctx.activeInput->caret + 1);
            break;
        case GLFW_KEY_HOME: moveTo(0); break;
        case GLFW_KEY_END:  moveTo(len); break;
        case GLFW_KEY_BACKSPACE:
            if (hasSelection(ctx.activeInput)) {
                deleteSelectionRange(ctx.activeInput);
            } else if (ctx.activeInput->caret > 0) {
                std::memmove(ctx.activeInput->text + ctx.activeInput->caret - 1,
                             ctx.activeInput->text + ctx.activeInput->caret,
                             (size_t)(len - ctx.activeInput->caret + 1)); // includes null
                ctx.activeInput->caret--; ctx.activeInput->selAnchor = ctx.activeInput->caret;
                resetBlink(ctx.activeInput);
            }
            break;
        case GLFW_KEY_DELETE:
            if (hasSelection(ctx.activeInput)) {
                deleteSelectionRange(ctx.activeInput);
            } else if (ctx.activeInput->caret < len) {
                std::memmove(ctx.activeInput->text + ctx.activeInput->caret,
                             ctx.activeInput->text + ctx.activeInput->caret + 1,
                             (size_t)(len - ctx.activeInput->caret)); // includes null
                resetBlink(ctx.activeInput);
            }
            break;
        default: break;
    }
}

inline void onKey(int key, int /*scancode*/, int action, int mods) {
    handleKey(key, action, mods);
}

} // namespace tinygui