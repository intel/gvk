
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

#include "gvk/system/input.hpp"

#include <array>

struct GLFWwindow;

namespace gvk {
namespace sys {

class Surface final
{
public:
    enum class CursorMode
    {
        Visible,
        Hidden,
        Disabled,
    };

    enum StatusFlagBits
    {
        CloseRequested = 1,
        ResizeOcurred  = 1 << 1,
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
        std::array<int, 2> position { 320, 180 };
        std::array<int, 2> extent { 1280, 720 };
        CursorMode cursorMode { CursorMode::Visible };
    };

    Surface() = default;
    static bool create(const CreateInfo* pCreateInfo, Surface* pSurface);
    Surface(Surface&& other) noexcept;
    Surface& operator=(Surface&& other) noexcept;
    ~Surface();
    void reset();
    operator bool() const;

    const Input& get_input() const;
    StatusFlags get_status() const;
    const std::array<int, 2>& get_extent() const;
    #if defined(_WIN32) || defined(_WIN64)
    void* get_hwnd() const;
    #endif
    static void update();

private:
#ifdef GVK_GLFW_ENABLED
    static void glfw_error_callback(int, const char*);
    static void glfw_window_close_callback(GLFWwindow*);
    static void glfw_framebuffer_size_callback(GLFWwindow*, int, int);
    static void glfw_key_callback(GLFWwindow*, int, int, int, int);
    static void glfw_cursor_pos_callback(GLFWwindow*, double, double);
    static void glfw_mouse_button_callback(GLFWwindow*, int, int, int);
    static void glfw_scroll_callback(GLFWwindow*, double, double);
#endif // GVK_GLFW_ENABLED

    Input mInput;
    StatusFlags mStatus{ };
    std::array<int, 2> mExtent{ };
    GLFWwindow* mpGlfwWindow{ nullptr };

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
};

} // namespace sys
} // namespace gvk
