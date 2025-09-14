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

struct Context {
    GLFWwindow* window;
    float mouseX, mouseY;
    bool mouseDown;
    bool mousePressed;
    char inputText[TINYGUI_MAX_TEXT];
    bool inputActive;

    // Caret/selection state
    int caret;            // insertion point [0..len]
    int selAnchor;        // selection anchor
    bool selecting;       // mouse-drag in progress
    double blinkStart;    // caret blink origin (seconds)
};

static Context ctx;

// ============== Helpers for text editing ==============
inline int textLen() { return (int)std::strlen(ctx.inputText); }

inline int clampIndex(int i) {
    int len = textLen();
    return std::max(0, std::min(i, len));
}

inline bool hasSelection() { return ctx.caret != ctx.selAnchor; }

inline void resetBlink() { ctx.blinkStart = glfwGetTime(); }

inline void setCaret(int pos, bool keepSelection) {
    pos = clampIndex(pos);
    ctx.caret = pos;
    if (!keepSelection) ctx.selAnchor = pos;
    resetBlink();
}

inline void deleteSelectionRange() {
    if (!hasSelection()) return;
    int a = std::min(ctx.selAnchor, ctx.caret);
    int b = std::max(ctx.selAnchor, ctx.caret);
    int len = textLen();
    std::memmove(ctx.inputText + a, ctx.inputText + b, (size_t)(len - b + 1)); // include null terminator
    ctx.caret = ctx.selAnchor = a;
    resetBlink();
}

// Compute per-character left/right x (in pixels after scale) and the right-most x.
// Returns length actually processed.
inline int computeGlyphBounds(const char* text, float scale,
                              float* lefts, float* rights, int maxChars,
                              float* outRightmost) {
    int len = (int)std::strlen(text);
    if (len > maxChars) len = maxChars;

    // 64 bytes per char (4 verts * 16 bytes)
    char vbuf[TINYGUI_MAX_TEXT * 64];
    int num_quads = stb_easy_font_print(0, 0, (char*)text, NULL, vbuf, sizeof(vbuf));
    (void)num_quads;

    float rightMost = 0.0f;
    for (int i = 0; i < len; ++i) {
        float* verts = (float*)vbuf + (i * 4) * 4; // 4 verts per quad, 4 floats each
        float minx = verts[0], maxx = verts[0];
        for (int v = 0; v < 4; ++v) {
            float vx = verts[v * 4 + 0];
            if (vx < minx) minx = vx;
            if (vx > maxx) maxx = vx;
        }
        lefts[i]  = minx * scale;
        rights[i] = maxx * scale;
        rightMost = rights[i];
    }
    if (outRightmost) *outRightmost = (len > 0 ? rightMost : 0.0f);
    return len;
}

// ==================== Initialization ====================
inline bool init(int w, int h, const char* title) {
    if (!glfwInit()) return false;
    ctx.window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!ctx.window) return false;
    glfwMakeContextCurrent(ctx.window);

    ctx.mouseX = ctx.mouseY = 0;
    ctx.mouseDown = ctx.mousePressed = false;
    ctx.inputText[0] = 0;
    ctx.inputActive = false;

    ctx.caret = ctx.selAnchor = 0;
    ctx.selecting = false;
    ctx.blinkStart = glfwGetTime();

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

inline void endFrame() { glfwSwapBuffers(ctx.window); }

// ==================== GUI Widgets ====================
inline void label(float x, float y, const char* text, float scale = TINYGUI_LABEL_SCALE) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);

    // Each char uses 64 bytes (4 verts * 16 bytes); allow generous buffer
    char buffer[16384];
    int num_quads = stb_easy_font_print(0, 0, (char*)text, NULL, buffer, sizeof(buffer));

    glColor3f(1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

inline bool button(float x, float y, float w, float h, const char* text) {
    // Hit-test in window coords (no Y flip)
    bool hovered = (ctx.mouseX >= x && ctx.mouseX <= x + w &&
                    ctx.mouseY >= y && ctx.mouseY <= y + h);

    glColor3f(hovered ? 0.7f : 0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

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
inline bool input(float x, float y, float w, float h, const char* hint = "") {
    // Activation and mouse interaction
    bool inside = (ctx.mouseX >= x && ctx.mouseX <= x + w &&
                   ctx.mouseY >= y && ctx.mouseY <= y + h);

    bool shiftDown = glfwGetKey(ctx.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                     glfwGetKey(ctx.window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    const float s = TINYGUI_LABEL_SCALE;
    const float padX = 5.0f, padY = 5.0f;
    const float lineH = 8.0f * s;

    // Precompute glyph bounds for current text
    float lefts[TINYGUI_MAX_TEXT]{}, rights[TINYGUI_MAX_TEXT]{};
    float textRight = 0.0f;
    int len = computeGlyphBounds(ctx.inputText, s, lefts, rights, TINYGUI_MAX_TEXT - 1, &textRight);

    auto caretXAt = [&](int idx) -> float {
        if (idx <= 0) return x + padX;
        if (idx >= len) return x + padX + textRight;
        return x + padX + lefts[idx];
    };

    auto indexFromX = [&](float mouseX) -> int {
        float lx = mouseX - (x + padX);
        if (lx <= 0.0f) return 0;
        if (lx >= textRight) return len;
        for (int i = 0; i < len; ++i) {
            float mid = (lefts[i] + rights[i]) * 0.5f;
            if (lx < mid) return i;
        }
        return len;
    };

    if (ctx.mousePressed && inside) {
        ctx.inputActive = true;
        int newCaret = indexFromX(ctx.mouseX);
        if (!shiftDown) ctx.selAnchor = newCaret;
        ctx.caret = newCaret;
        ctx.selecting = true;
        glfwFocusWindow(ctx.window);
        resetBlink();
    } else if (ctx.mousePressed && !inside) {
        ctx.inputActive = false;
        ctx.selecting = false;
    } else if (!ctx.mouseDown) {
        ctx.selecting = false;
    }

    // Drag selection
    if (ctx.inputActive && ctx.selecting) {
        ctx.caret = indexFromX(ctx.mouseX);
        resetBlink();
    }

    // Draw box
    glColor3f(ctx.inputActive ? 0.6f : 0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    // Draw selection (behind text)
    if (ctx.inputActive && hasSelection()) {
        int a = std::min(ctx.selAnchor, ctx.caret);
        int b = std::max(ctx.selAnchor, ctx.caret);
        float selX0 = caretXAt(a);
        float selX1 = caretXAt(b);

        glColor3f(0.25f, 0.45f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(selX0, y + padY);
        glVertex2f(selX1, y + padY);
        glVertex2f(selX1, y + padY + lineH);
        glVertex2f(selX0, y + padY + lineH);
        glEnd();
    }

    // Draw text or hint
    if (ctx.inputText[0] != 0) {
        glColor3f(1, 1, 1);
        label(x + padX, y + padY, ctx.inputText, s);
    } else if (hint[0] != 0) {
        glColor3f(0.7f, 0.7f, 0.7f);
        label(x + padX, y + padY, hint, s);
        glColor3f(1, 1, 1);
    }

    // Draw caret (blinking)
    if (ctx.inputActive) {
        double t = glfwGetTime();
        bool showCaret = fmod(t - ctx.blinkStart, 1.0) < 0.5;
        if (showCaret) {
            float cx = caretXAt(ctx.caret);
            glColor3f(1, 1, 1);
            glBegin(GL_LINES);
            glVertex2f(cx, y + padY);
            glVertex2f(cx, y + padY + lineH);
            glEnd();
        }
    }

    return ctx.inputActive;
}

// ==================== Character and Key handling ====================
inline void addCharToInput(unsigned int codepoint) {
    if (!ctx.inputActive) return;
    if (codepoint < 32 || codepoint > 126) return; // printable ASCII only
    if (hasSelection()) deleteSelectionRange();

    int len = textLen();
    if (len >= TINYGUI_MAX_TEXT - 1) return;

    int pos = clampIndex(ctx.caret);
    std::memmove(ctx.inputText + pos + 1, ctx.inputText + pos, (size_t)(len - pos + 1)); // includes null
    ctx.inputText[pos] = (char)codepoint;
    ctx.caret = ctx.selAnchor = pos + 1;
    resetBlink();
}

inline void onChar(unsigned int codepoint) { addCharToInput(codepoint); }

inline void handleKey(int key, int action, int mods) {
    if (!ctx.inputActive) return;
    if (!(action == GLFW_PRESS || action == GLFW_REPEAT)) return;

    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    int len = textLen();

    auto moveTo = [&](int pos) {
        pos = clampIndex(pos);
        if (!shift) ctx.selAnchor = pos;
        ctx.caret = pos;
        resetBlink();
    };

    switch (key) {
        case GLFW_KEY_LEFT:
            if (hasSelection() && !shift) moveTo(std::min(ctx.selAnchor, ctx.caret));
            else moveTo(ctx.caret - 1);
            break;
        case GLFW_KEY_RIGHT:
            if (hasSelection() && !shift) moveTo(std::max(ctx.selAnchor, ctx.caret));
            else moveTo(ctx.caret + 1);
            break;
        case GLFW_KEY_HOME: moveTo(0); break;
        case GLFW_KEY_END:  moveTo(len); break;
        case GLFW_KEY_BACKSPACE:
            if (hasSelection()) {
                deleteSelectionRange();
            } else if (ctx.caret > 0) {
                std::memmove(ctx.inputText + ctx.caret - 1,
                             ctx.inputText + ctx.caret,
                             (size_t)(len - ctx.caret + 1)); // includes null
                ctx.caret--; ctx.selAnchor = ctx.caret;
                resetBlink();
            }
            break;
        case GLFW_KEY_DELETE:
            if (hasSelection()) {
                deleteSelectionRange();
            } else if (ctx.caret < len) {
                std::memmove(ctx.inputText + ctx.caret,
                             ctx.inputText + ctx.caret + 1,
                             (size_t)(len - ctx.caret)); // includes null
                resetBlink();
            }
            break;
        default: break;
    }
}

inline void onKey(int key, int /*scancode*/, int action, int mods) {
    handleKey(key, action, mods);
}

} // namespace tinygui