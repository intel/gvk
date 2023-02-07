
/*******************************************************************************

MIT License

Copyright (c) Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#pragma once

#include "gvk-system/input.hpp"

#include <array>
#include <unordered_map>
#include <vector>

struct GLFWcursor;
struct GLFWwindow;

namespace gvk {
namespace system {

class Surface final
{
public:
    enum class CursorMode
    {
        Visible,
        Hidden,
        Disabled,
    };

    enum class CursorType
    {
        Arrow,
        IBeam,
        Hand,
        Crosshair,
        ResizeEW,
        ResizeNS,
        ResizeNWSE,
        ResizeNESW,
        ResizeAll,
        NotAllowed,
    };

    enum StatusFlagBits
    {
        CloseRequested = 1,
        Resized        = 1 << 1,
        GainedFocus    = 1 << 2,
        LostFocus      = 1 << 3,
    };

    using StatusFlags = uint32_t;

    struct CreateInfo final
    {
        enum FlagBits
        {
            Decorated   = 1,
            Fullscreen  = 1 << 1,
            Resizable   = 1 << 2,
            Visible     = 1 << 3,
            Transparent = 1 << 4,
        };

        uint32_t flags { Decorated | Resizable | Visible };
        const char* pTitle { nullptr };
        std::array<int32_t, 2> position { 320, 180 };
        std::array<int32_t, 2> extent { 1280, 720 };
        CursorMode cursorMode { CursorMode::Visible };
    };

    Surface() = default;
    static bool create(const CreateInfo* pCreateInfo, Surface* pSurface);
    Surface(Surface&& other) noexcept;
    Surface& operator=(Surface&& other) noexcept;
    ~Surface();
    void reset();
    operator bool() const;

    CursorMode get_cursor_mode() const;
    void set_cursor_mode(CursorMode cursorMode);
    void set_cursor_type(CursorType cursorType);
    const Input& get_input() const;
    StatusFlags get_status() const;
    const std::array<int32_t, 2>& get_extent() const;
    const std::vector<uint32_t>& get_text_stream() const;
#if defined(__linux__)
    void* get_display() const;
    unsigned long get_window() const;
#endif
#if defined(_WIN32) || defined(_WIN64)
    void* get_hwnd() const;
#endif
    static void update();

private:
    static void glfw_error_callback(int, const char*);
    static void glfw_window_close_callback(GLFWwindow*);
    static void glfw_framebuffer_size_callback(GLFWwindow*, int, int);
    static void glfw_char_callback(GLFWwindow*, unsigned int);
    static void glfw_key_callback(GLFWwindow*, int, int, int, int);
    static void glfw_cursor_pos_callback(GLFWwindow*, double, double);
    static void glfw_mouse_button_callback(GLFWwindow*, int, int, int);
    static void glfw_scroll_callback(GLFWwindow*, double, double);
    static void glfw_window_focus_callback(GLFWwindow*, int);
    static std::unordered_map<CursorType, GLFWcursor*> smCursors;

    Input mInput;
    StatusFlags mStatus{ };
    std::array<int32_t, 2> mExtent{ };
    std::vector<uint32_t> mTextStream;
    GLFWwindow* mpGlfwWindow { nullptr };

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
};

} // namespace system
} // namespace gvk
