
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

#include "gvk/system/surface.hpp"

#include <cassert>

#ifdef GVK_GLFW_ENABLED
#if defined(_WIN32) || defined(_WIN64)
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#endif

#if defined(__linux__)
#ifndef GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif
#endif

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#endif // GVK_GLFW_ENABLED

#include <cassert>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <utility>

namespace gvk {
namespace sys {

#ifdef GVK_GLFW_ENABLED

class GlfwWindowSet final
{
public:
    template <typename FunctionType>
    inline void access(FunctionType accessGlfwWindowSet)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        accessGlfwWindowSet(mGlfwWindows);
    }

    static inline GlfwWindowSet& instance()
    {
        static GlfwWindowSet sGlfwWindowSet;
        return sGlfwWindowSet;
    }

private:
    std::mutex mMutex;
    std::set<GLFWwindow*> mGlfwWindows;
};

bool Surface::create(const CreateInfo* pCreateInfo, Surface* pSurface)
{
    if (pCreateInfo && pSurface) {
        pSurface->reset();
        GlfwWindowSet::instance().access(
            [&](std::set<GLFWwindow*>& glfwWindows)
            {
                if (glfwWindows.empty()) {
                    glfwSetErrorCallback(glfw_error_callback);
                    glfwInit();
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                }
                glfwWindowHint(GLFW_DECORATED, pCreateInfo->flags & Surface::CreateInfo::Decorated);
                glfwWindowHint(GLFW_RESIZABLE, pCreateInfo->flags & Surface::CreateInfo::Resizable);
                glfwWindowHint(GLFW_VISIBLE, pCreateInfo->flags & Surface::CreateInfo::Visible);
                glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, pCreateInfo->flags & Surface::CreateInfo::Transparent);
                pSurface->mpGlfwWindow = glfwCreateWindow(
                    pCreateInfo->extent[0],
                    pCreateInfo->extent[1],
                    pCreateInfo->pTitle ? pCreateInfo->pTitle : "Intel GVK",
                    pCreateInfo->flags & Surface::CreateInfo::Fullscreen ? glfwGetPrimaryMonitor() : nullptr,
                    nullptr
                );
                if (pSurface->mpGlfwWindow) {
                    glfwSetWindowUserPointer(pSurface->mpGlfwWindow, pSurface);
                    glfwSetWindowCloseCallback(pSurface->mpGlfwWindow, glfw_window_close_callback);
                    glfwSetFramebufferSizeCallback(pSurface->mpGlfwWindow, glfw_framebuffer_size_callback);
                    glfwSetKeyCallback(pSurface->mpGlfwWindow, glfw_key_callback);
                    glfwSetCursorPosCallback(pSurface->mpGlfwWindow, glfw_cursor_pos_callback);
                    glfwSetMouseButtonCallback(pSurface->mpGlfwWindow, glfw_mouse_button_callback);
                    glfwSetScrollCallback(pSurface->mpGlfwWindow, glfw_scroll_callback);
                    glfwGetFramebufferSize(pSurface->mpGlfwWindow, &pSurface->mExtent[0], &pSurface->mExtent[1]);
                    glfwWindows.insert(pSurface->mpGlfwWindow);
                }
            }
        );
    }
    return pSurface && pSurface->mpGlfwWindow != nullptr;
}

Surface::Surface(Surface&& other) noexcept
{
    *this = std::move(other);
}

Surface& Surface::operator=(Surface&& other) noexcept
{
    if (this != &other) {
        mInput = std::move(other.mInput);
        mStatus = std::move(other.mStatus);
        mExtent = std::move(other.mExtent);
        mpGlfwWindow = std::move(other.mpGlfwWindow);
        glfwSetWindowUserPointer(mpGlfwWindow, this);
        other.mpGlfwWindow = nullptr;
    }
    return *this;
}

Surface::~Surface()
{
    reset();
}

void Surface::reset()
{
    if (mpGlfwWindow) {
        GlfwWindowSet::instance().access(
            [&](std::set<GLFWwindow*>& glfwWindows)
            {
                glfwDestroyWindow(mpGlfwWindow);
                glfwWindows.erase(mpGlfwWindow);
                if (glfwWindows.empty()) {
                    glfwTerminate();
                }
            }
        );
    }
    mInput = { };
    mStatus = 0;
    mExtent = { };
    mpGlfwWindow = nullptr;
}

Surface::operator bool() const
{
    return mpGlfwWindow != nullptr;
}

const Input& Surface::get_input() const
{
    return mInput;
}

Surface::StatusFlags Surface::get_status() const
{
    return mStatus;
}

const std::array<int, 2>& Surface::get_extent() const
{
    return mExtent;
}

#if defined(__linux__)
void* Surface::get_display() const
{
    assert(mpGlfwWindow);
    return glfwGetX11Display();
}

unsigned long Surface::get_window() const
{
    assert(mpGlfwWindow);
    return glfwGetX11Window(mpGlfwWindow);
}
#endif

#if defined(_WIN32) || defined(_WIN64)
void* Surface::get_hwnd() const
{
    assert(mpGlfwWindow);
    return glfwGetWin32Window(mpGlfwWindow);
}
#endif

void Surface::update()
{
    GlfwWindowSet::instance().access(
        [&](std::set<GLFWwindow*>& glfwWindows)
        {
            for (auto glfwWindow : glfwWindows) {
                auto pSurface = (Surface*)glfwGetWindowUserPointer(glfwWindow);
                assert(pSurface);
                pSurface->mStatus = 0;
                pSurface->mInput.update();
            }
            glfwPollEvents();
        }
    );
}

Key glfw_to_gvk_key(int glfwKey)
{
    switch (glfwKey) {
    case GLFW_KEY_SPACE        : return Key::SpaceBar;
    case GLFW_KEY_APOSTROPHE   : return Key::OEM_Quote;
    case GLFW_KEY_COMMA        : return Key::OEM_Comma;
    case GLFW_KEY_MINUS        : return Key::OEM_Minus;
    case GLFW_KEY_PERIOD       : return Key::OEM_Period;
    case GLFW_KEY_SLASH        : return Key::OEM_ForwardSlash;
    case GLFW_KEY_0            : return Key::Zero;
    case GLFW_KEY_1            : return Key::One;
    case GLFW_KEY_2            : return Key::Two;
    case GLFW_KEY_3            : return Key::Three;
    case GLFW_KEY_4            : return Key::Four;
    case GLFW_KEY_5            : return Key::Five;
    case GLFW_KEY_6            : return Key::Six;
    case GLFW_KEY_7            : return Key::Seven;
    case GLFW_KEY_8            : return Key::Eight;
    case GLFW_KEY_9            : return Key::Nine;
    case GLFW_KEY_SEMICOLON    : return Key::OEM_SemiColon;
    case GLFW_KEY_EQUAL        : return Key::OEM_Plus;
    case GLFW_KEY_A            : return Key::A;
    case GLFW_KEY_B            : return Key::B;
    case GLFW_KEY_C            : return Key::C;
    case GLFW_KEY_D            : return Key::D;
    case GLFW_KEY_E            : return Key::E;
    case GLFW_KEY_F            : return Key::F;
    case GLFW_KEY_G            : return Key::G;
    case GLFW_KEY_H            : return Key::H;
    case GLFW_KEY_I            : return Key::I;
    case GLFW_KEY_J            : return Key::J;
    case GLFW_KEY_K            : return Key::K;
    case GLFW_KEY_L            : return Key::L;
    case GLFW_KEY_M            : return Key::M;
    case GLFW_KEY_N            : return Key::N;
    case GLFW_KEY_O            : return Key::O;
    case GLFW_KEY_P            : return Key::P;
    case GLFW_KEY_Q            : return Key::Q;
    case GLFW_KEY_R            : return Key::R;
    case GLFW_KEY_S            : return Key::S;
    case GLFW_KEY_T            : return Key::T;
    case GLFW_KEY_U            : return Key::U;
    case GLFW_KEY_V            : return Key::V;
    case GLFW_KEY_W            : return Key::W;
    case GLFW_KEY_X            : return Key::X;
    case GLFW_KEY_Y            : return Key::Y;
    case GLFW_KEY_Z            : return Key::Z;
    case GLFW_KEY_LEFT_BRACKET : return Key::OEM_OpenBracket;
    case GLFW_KEY_BACKSLASH    : return Key::OEM_BackSlash;
    case GLFW_KEY_RIGHT_BRACKET: return Key::OEM_CloseBracket;
    case GLFW_KEY_GRAVE_ACCENT : return Key::OEM_Tilde;
    case GLFW_KEY_WORLD_1      : return Key::Unknown;
    case GLFW_KEY_WORLD_2      : return Key::Unknown;
    case GLFW_KEY_ESCAPE       : return Key::Escape;
    case GLFW_KEY_ENTER        : return Key::Enter;
    case GLFW_KEY_TAB          : return Key::Tab;
    case GLFW_KEY_BACKSPACE    : return Key::Backspace;
    case GLFW_KEY_INSERT       : return Key::Insert;
    case GLFW_KEY_DELETE       : return Key::Delete;
    case GLFW_KEY_RIGHT        : return Key::RightArrow;
    case GLFW_KEY_LEFT         : return Key::LeftArrow;
    case GLFW_KEY_DOWN         : return Key::DownArrow;
    case GLFW_KEY_UP           : return Key::UpArrow;
    case GLFW_KEY_PAGE_UP      : return Key::PageUp;
    case GLFW_KEY_PAGE_DOWN    : return Key::PageDown;
    case GLFW_KEY_HOME         : return Key::Home;
    case GLFW_KEY_END          : return Key::End;
    case GLFW_KEY_CAPS_LOCK    : return Key::CapsLock;
    case GLFW_KEY_SCROLL_LOCK  : return Key::ScrollLock;
    case GLFW_KEY_NUM_LOCK     : return Key::NumLock;
    case GLFW_KEY_PRINT_SCREEN : return Key::PrintScreen;
    case GLFW_KEY_PAUSE        : return Key::Pause;
    case GLFW_KEY_F1           : return Key::F1;
    case GLFW_KEY_F2           : return Key::F2;
    case GLFW_KEY_F3           : return Key::F3;
    case GLFW_KEY_F4           : return Key::F4;
    case GLFW_KEY_F5           : return Key::F5;
    case GLFW_KEY_F6           : return Key::F6;
    case GLFW_KEY_F7           : return Key::F7;
    case GLFW_KEY_F8           : return Key::F8;
    case GLFW_KEY_F9           : return Key::F9;
    case GLFW_KEY_F10          : return Key::F10;
    case GLFW_KEY_F11          : return Key::F11;
    case GLFW_KEY_F12          : return Key::F12;
    case GLFW_KEY_F13          : return Key::F13;
    case GLFW_KEY_F14          : return Key::F14;
    case GLFW_KEY_F15          : return Key::F15;
    case GLFW_KEY_F16          : return Key::F16;
    case GLFW_KEY_F17          : return Key::F17;
    case GLFW_KEY_F18          : return Key::F18;
    case GLFW_KEY_F19          : return Key::F19;
    case GLFW_KEY_F20          : return Key::F20;
    case GLFW_KEY_F21          : return Key::F21;
    case GLFW_KEY_F22          : return Key::F22;
    case GLFW_KEY_F23          : return Key::F23;
    case GLFW_KEY_F24          : return Key::F24;
    case GLFW_KEY_F25          : return Key::Unknown;
    case GLFW_KEY_KP_0         : return Key::NumPad0;
    case GLFW_KEY_KP_1         : return Key::NumPad1;
    case GLFW_KEY_KP_2         : return Key::NumPad2;
    case GLFW_KEY_KP_3         : return Key::NumPad3;
    case GLFW_KEY_KP_4         : return Key::NumPad4;
    case GLFW_KEY_KP_5         : return Key::NumPad5;
    case GLFW_KEY_KP_6         : return Key::NumPad6;
    case GLFW_KEY_KP_7         : return Key::NumPad7;
    case GLFW_KEY_KP_8         : return Key::NumPad8;
    case GLFW_KEY_KP_9         : return Key::NumPad9;
    case GLFW_KEY_KP_DECIMAL   : return Key::Decimal;
    case GLFW_KEY_KP_DIVIDE    : return Key::Divide;
    case GLFW_KEY_KP_MULTIPLY  : return Key::Multiply;
    case GLFW_KEY_KP_SUBTRACT  : return Key::Subtract;
    case GLFW_KEY_KP_ADD       : return Key::Add;
    case GLFW_KEY_KP_ENTER     : return Key::Enter;
    case GLFW_KEY_KP_EQUAL     : return Key::Unknown;
    case GLFW_KEY_LEFT_SHIFT   : return Key::LeftShift;
    case GLFW_KEY_LEFT_CONTROL : return Key::LeftControl;
    case GLFW_KEY_LEFT_ALT     : return Key::Alt;
    case GLFW_KEY_LEFT_SUPER   : return Key::Unknown;
    case GLFW_KEY_RIGHT_SHIFT  : return Key::RightShift;
    case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
    case GLFW_KEY_RIGHT_ALT    : return Key::Alt;
    case GLFW_KEY_RIGHT_SUPER  : return Key::Unknown;
    case GLFW_KEY_MENU         : return Key::Unknown;
    default                    : return Key::Unknown;
    }
}

Mouse::Button glfw_to_gvk_mouse_button(int glfwMouseButton)
{
    switch (glfwMouseButton) {
    case GLFW_MOUSE_BUTTON_LEFT  : return Mouse::Button::Left;
    case GLFW_MOUSE_BUTTON_RIGHT : return Mouse::Button::Right;
    case GLFW_MOUSE_BUTTON_MIDDLE: return Mouse::Button::Middle;
    case GLFW_MOUSE_BUTTON_4     : return Mouse::Button::X1;
    case GLFW_MOUSE_BUTTON_5     : return Mouse::Button::X2;
    case GLFW_MOUSE_BUTTON_6     : return Mouse::Button::Unknown;
    case GLFW_MOUSE_BUTTON_7     : return Mouse::Button::Unknown;
    case GLFW_MOUSE_BUTTON_LAST  : return Mouse::Button::Unknown;
    default                      : return Mouse::Button::Unknown;
    }
}

void Surface::glfw_error_callback(int error, const char* pMessage)
{
    std::cerr << "GLFW Error [" << error << "] : " << (pMessage ? pMessage : "Unknown") << std::endl;
    assert(false);
}

void Surface::glfw_window_close_callback(GLFWwindow* glfwWindow)
{
    auto pSurface = (Surface*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurface);
    pSurface->mStatus |= Surface::CloseRequested;
}

void Surface::glfw_framebuffer_size_callback(GLFWwindow* glfwWindow, int width, int height)
{
    auto pSurface = (Surface*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurface);
    pSurface->mExtent = { width, height };
    pSurface->mStatus |= Surface::ResizeOcurred;
}

void Surface::glfw_key_callback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    auto pSurface = (Surface*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurface);
    pSurface->mInput.keyboard.staged[(size_t)glfw_to_gvk_key(key)] = action == GLFW_PRESS || action == GLFW_REPEAT;
}

void Surface::glfw_cursor_pos_callback(GLFWwindow* glfwWindow, double xOffset, double yOffset)
{
    auto pSurface = (Surface*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurface);
    pSurface->mInput.mouse.position.staged = { (float)xOffset, (float)yOffset };
}

void Surface::glfw_mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods)
{
    (void)mods;
    auto pSurface = (Surface*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurface);
    pSurface->mInput.mouse.buttons.staged[(size_t)glfw_to_gvk_mouse_button(button)] = action == GLFW_PRESS || action == GLFW_REPEAT;
}

void Surface::glfw_scroll_callback(GLFWwindow* glfwWindow, double xOffset, double yOffset)
{
    auto pSurface = (Surface*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurface);
    pSurface->mInput.mouse.scroll.staged[0] += (float)xOffset;
    pSurface->mInput.mouse.scroll.staged[1] += (float)yOffset;
}

#else // GVK_GLFW_ENABLED
bool Surface::create(const CreateInfo*, Surface*) { assert(false && "TODO : Generic surface support; gvk::sys::Surface currently requires that GVK be built with GVK_GLFW_ENABLED"); return false; }
Surface::Surface(Surface&&) noexcept { }
Surface& Surface::operator=(Surface&&) noexcept { return *this; }
Surface::~Surface() { }
void Surface::reset() { }
Surface::operator bool() const { assert(false && "TODO : Generic surface support; gvk::sys::Surface currently requires that GVK be built with GVK_GLFW_ENABLED"); return false; }
const Input& Surface::get_input() const { assert(false && "TODO : Generic surface support; gvk::sys::Surface currently requires that GVK be built with GVK_GLFW_ENABLED"); return mInput; }
Surface::StatusFlags Surface::get_status() const { assert(false && "TODO : Generic surface support; gvk::sys::Surface currently requires that GVK be built with GVK_GLFW_ENABLED"); return { }; }
const std::array<int, 2>& Surface::get_extent() const { assert(false && "TODO : Generic surface support; gvk::sys::Surface currently requires that GVK be built with GVK_GLFW_ENABLED"); return mExtent; }
#if defined(_WIN32) || defined(_WIN64)
void* Surface::get_hwnd() const { assert(false && "TODO : Generic surface support; gvk::sys::Surface currently requires that GVK be built with GVK_GLFW_ENABLED"); return nullptr; }
#endif
void Surface::update() { assert(false && "TODO : Generic surface support; gvk::sys::Surface currently requires that GVK be built with GVK_GLFW_ENABLED"); }
#endif // GVK_GLFW_ENABLED

} // namespace sys
} // namespace gvk
