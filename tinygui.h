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

#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>

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
    
    InputState() {
        text[0] = 0;
        caret = selAnchor = 0;
        selecting = false;
        blinkStart = 0.0;
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
    float tx = x + (w - textW) * 0.5f;
    float ty = y + (h - textH) * 0.5f;
    label(tx, ty, text, s);

    return hovered && ctx.mousePressed;
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

    // Precompute character widths for current text
    float charWidths[TINYGUI_MAX_TEXT]{};
    float cumWidths[TINYGUI_MAX_TEXT]{};
    float totalTextWidth = 0.0f;
    int len = computeCharWidths(inputState.text, s, charWidths, cumWidths, TINYGUI_MAX_TEXT - 1, &totalTextWidth);

    auto caretXAt = [&](int idx) -> float {
        if (idx <= 0) return x + padX;
        if (idx >= len) return x + padX + totalTextWidth;
        return x + padX + cumWidths[idx];
    };

    auto indexFromX = [&](float mouseX) -> int {
        float lx = mouseX - (x + padX);
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

    // Draw selection (behind text)
    if (isActive && hasSelection(&inputState)) {
        int a = std::min(inputState.selAnchor, inputState.caret);
        int b = std::max(inputState.selAnchor, inputState.caret);
        float selX0 = caretXAt(a);
        float selX1 = caretXAt(b);

        drawRect(selX0, y + padY, selX1 - selX0, lineH, THEME_SELECTION);
    }

    // Draw text or hint
    if (inputState.text[0] != 0) {
        label(x + padX, y + padY, inputState.text, s, THEME_TEXT);
    } else if (hint[0] != 0) {
        label(x + padX, y + padY, hint, s, THEME_TEXT_DIM);
    }

    // Draw caret (blinking)
    if (isActive) {
        double t = glfwGetTime();
        bool showCaret = fmod(t - inputState.blinkStart, 1.0) < 0.5;
        if (showCaret) {
            float cx = caretXAt(inputState.caret);
            drawLine(cx, y + padY, cx, y + padY + lineH, THEME_TEXT);
        }
    }

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
    
    // Draw track
    float trackY = y + h * 0.4f;
    float trackH = h * 0.2f;
    drawRect(x, trackY, w, trackH, THEME_INPUT);
    
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
    
    // Draw background
    drawRect(x, y, w, h, THEME_INPUT);
    
    // Draw progress fill
    if (progress > 0.0f) {
        drawRect(x, y, w * progress, h, fillColor);
    }
    
    // Draw border
    drawRectOutline(x, y, w, h, THEME_TEXT, 1.0f);
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

// Define endFrame here after menu functions are available
inline void endFrame() { 
    // Automatically draw dropdown menus at the end of frame
    ctx.pendingMenuResult = standardMenuDropdowns();
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