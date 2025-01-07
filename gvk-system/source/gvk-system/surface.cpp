
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

#include "gvk-system/surface.hpp"

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#if defined(__linux__)
#ifndef GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#endif

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <cassert>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <utility>

namespace gvk {
namespace system {

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

static void glfw_error_callback(int, const char*);
static void glfw_window_close_callback(GLFWwindow*);
static void glfw_window_size_callback(GLFWwindow*, int, int);
static void glfw_framebuffer_size_callback(GLFWwindow*, int, int);
static void glfw_char_callback(GLFWwindow*, unsigned int);
static void glfw_key_callback(GLFWwindow*, int, int, int, int);
static void glfw_cursor_pos_callback(GLFWwindow*, double, double);
static void glfw_mouse_button_callback(GLFWwindow*, int, int, int);
static void glfw_scroll_callback(GLFWwindow*, double, double);
static void glfw_window_focus_callback(GLFWwindow*, int);
static std::unordered_map<Surface::CursorType, GLFWcursor*> sCursors;

int32_t Surface::create(const CreateInfo* pCreateInfo, Surface* pSurface)
{
    assert(pCreateInfo);
    assert(pSurface);
    // NOTE : Creating new Reference here because the Surface::ControlBlock lifetime
    //  manages GLFW initialization and termination.
    Reference<Surface::ControlBlock> reference(newref);
    glfwWindowHint(GLFW_DECORATED, pCreateInfo->flags & Surface::CreateInfo::Decorated);
    glfwWindowHint(GLFW_RESIZABLE, pCreateInfo->flags & Surface::CreateInfo::Resizable);
    glfwWindowHint(GLFW_VISIBLE, pCreateInfo->flags & Surface::CreateInfo::Visible);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, pCreateInfo->flags & Surface::CreateInfo::Transparent);
    auto pGlfwWindow = glfwCreateWindow(
        pCreateInfo->extent[0],
        pCreateInfo->extent[1],
        pCreateInfo->pTitle,
        pCreateInfo->flags & Surface::CreateInfo::Fullscreen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr
    );
    if (pGlfwWindow) {
        if (pCreateInfo->position[0] || pCreateInfo->position[1]) {
            glfwSetWindowPos(pGlfwWindow, pCreateInfo->position[0], pCreateInfo->position[1]);
        }
        glfwSetWindowUserPointer(pGlfwWindow, &reference.get_obj());
        glfwSetWindowCloseCallback(pGlfwWindow, glfw_window_close_callback);
        glfwSetWindowSizeCallback(pGlfwWindow, glfw_window_size_callback);
        glfwSetFramebufferSizeCallback(pGlfwWindow, glfw_framebuffer_size_callback);
        glfwSetCharCallback(pGlfwWindow, glfw_char_callback);
        glfwSetKeyCallback(pGlfwWindow, glfw_key_callback);
        glfwSetCursorPosCallback(pGlfwWindow, glfw_cursor_pos_callback);
        glfwSetMouseButtonCallback(pGlfwWindow, glfw_mouse_button_callback);
        glfwSetScrollCallback(pGlfwWindow, glfw_scroll_callback);
        glfwSetWindowFocusCallback(pGlfwWindow, glfw_window_focus_callback);
        GlfwWindowSet::instance().access(
            [&](std::set<GLFWwindow*>& glfwWindows)
            {
                glfwWindows.insert(pGlfwWindow);
            }
        );
        if (pCreateInfo->pTitle) {
            reference->mTitle = pCreateInfo->pTitle;
        }
        reference->mpWindowHandle = pGlfwWindow;
        pSurface->mReference = reference;
        return 0; // VK_SUCCESS
    }
    return -3; // VK_ERROR_INITIALIZATION_FAILED
}

void Surface::update()
{
    GlfwWindowSet::instance().access(
        [&](std::set<GLFWwindow*>& glfwWindows)
        {
            for (auto glfwWindow : glfwWindows) {
                auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
                assert(pSurfaceControlBlock);
                pSurfaceControlBlock->mStatus = 0;
                pSurfaceControlBlock->mInput.update();
                pSurfaceControlBlock->mTextStream.clear();
            }
            glfwPollEvents();
        }
    );
}

void Surface::get_window_position(int32_t* pX, int32_t* pY) const
{
    assert(mReference);
    int x = 0;
    int y = 0;
    glfwGetWindowPos((GLFWwindow*)mReference->mpWindowHandle, &x, &y);
    if (pX) {
        *pX = x;
    }
    if (pY) {
        *pY = y;
    }
}

void Surface::get_window_extent(int32_t* pWidth, int32_t* pHeight) const
{
    assert(mReference);
    int width = 0;
    int height = 0;
    glfwGetWindowSize((GLFWwindow*)mReference->mpWindowHandle, &width, &height);
    if (pWidth) {
        *pWidth = width;
    }
    if (pHeight) {
        *pHeight = height;
    }
}

void Surface::set_window_extent(const std::array<int32_t, 2>& extent)
{
    assert(mReference);
    glfwSetWindowSize((GLFWwindow*)mReference->mpWindowHandle, extent[0], extent[1]);
}

Surface::ControlBlock::ControlBlock()
{
    GlfwWindowSet::instance().access(
        [&](std::set<GLFWwindow*>& glfwWindows)
        {
            if (glfwWindows.empty()) {
                glfwSetErrorCallback(glfw_error_callback);
                glfwInit();
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                sCursors[CursorType::Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                sCursors[CursorType::IBeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
                sCursors[CursorType::Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
                sCursors[CursorType::Crosshair] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
                sCursors[CursorType::ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
                sCursors[CursorType::ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
                sCursors[CursorType::ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                sCursors[CursorType::ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                sCursors[CursorType::ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                sCursors[CursorType::NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            }
        }
    );
}

Surface::ControlBlock::~ControlBlock()
{
    GlfwWindowSet::instance().access(
        [&](std::set<GLFWwindow*>& glfwWindows)
        {
            glfwDestroyWindow((GLFWwindow*)mpWindowHandle);
            glfwWindows.erase((GLFWwindow*)mpWindowHandle);
            if (glfwWindows.empty()) {
                for (auto cursorItr : sCursors) {
                    glfwDestroyCursor(cursorItr.second);
                }
                sCursors.clear();
                glfwTerminate();
            }
        }
    );
}

const std::string& Surface::get_title() const
{
    assert(mReference);
    return mReference->mTitle;
}

void Surface::set_title(const std::string& title)
{
    assert(mReference);
    mReference->mTitle = title;
    glfwSetWindowTitle((GLFWwindow*)mReference->mpWindowHandle, mReference->mTitle.c_str());
}

const Surface::CursorMode& Surface::get_cursor_mode() const
{
    assert(mReference);
    auto& mutableReference = const_cast<decltype(mReference)&>(mReference);
    switch (glfwGetInputMode((GLFWwindow*)mutableReference->mpWindowHandle, GLFW_CURSOR)) {
    case GLFW_CURSOR_NORMAL: { mutableReference->mCursorMode = CursorMode::Visible; } break;
    case GLFW_CURSOR_HIDDEN: { mutableReference->mCursorMode = CursorMode::Hidden; } break;
    case GLFW_CURSOR_DISABLED: { mutableReference->mCursorMode = CursorMode::Disabled; } break;
    default: { assert(false && "Unserviced GFLW cursor mode; gvk maintenance required"); } break;
    }
    return mutableReference->mCursorMode;
}

void Surface::set_cursor_mode(CursorMode cursorMode)
{
    assert(mReference);
    auto pGlfwWindow = (GLFWwindow*)mReference->mpWindowHandle;
    switch (cursorMode) {
    case CursorMode::Visible: { glfwSetInputMode(pGlfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL); } break;
    case CursorMode::Hidden: { glfwSetInputMode(pGlfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); } break;
    case CursorMode::Disabled: { glfwSetInputMode(pGlfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); } break;
    default: { assert(false); } break;
    }
}

void Surface::set_cursor_type(CursorType cursorType)
{
    assert(mReference);
    auto cursorItr = sCursors.find(cursorType);
    if (cursorItr == sCursors.end()) {
        cursorItr = sCursors.find(CursorType::Arrow);
    }
    assert(cursorItr != sCursors.end());
    assert(cursorItr->second);
    glfwSetCursor((GLFWwindow*)mReference->mpWindowHandle, cursorItr->second);
}

const Surface::PlatformInfo& Surface::get_platform_info() const
{
    assert(mReference);
    auto& mutableReference = const_cast<decltype(mReference)&>(mReference);
#if defined(__linux__)
    mutableReference->mPlatformInfo.x11Display = glfwGetX11Display();
    mutableReference->mPlatformInfo.x11Window = glfwGetX11Window((GLFWwindow*)mReference->mpWindowHandle);
#endif
#if defined(_WIN32) || defined(_WIN64)
    mutableReference->mPlatformInfo.hwnd = glfwGetWin32Window((GLFWwindow*)mReference->mpWindowHandle);
#endif
    return mReference->mPlatformInfo;
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
    case GLFW_KEY_KP_EQUAL     : return Key::Enter;
    case GLFW_KEY_LEFT_SHIFT   : return Key::LeftShift;
    case GLFW_KEY_LEFT_CONTROL : return Key::LeftControl;
    case GLFW_KEY_LEFT_ALT     : return Key::LeftAlt;
    case GLFW_KEY_LEFT_SUPER   : return Key::LeftWindow;
    case GLFW_KEY_RIGHT_SHIFT  : return Key::RightShift;
    case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
    case GLFW_KEY_RIGHT_ALT    : return Key::RightAlt;
    case GLFW_KEY_RIGHT_SUPER  : return Key::RightWindow;
    case GLFW_KEY_MENU         : return Key::Menu;
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

void glfw_error_callback(int error, const char* pMessage)
{
    std::cerr << "GLFW Error [" << error << "] : " << (pMessage ? pMessage : "Unknown") << std::endl;
    assert(false);
}

void glfw_window_close_callback(GLFWwindow* glfwWindow)
{
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mStatus |= Surface::CloseRequested;
}

void glfw_window_size_callback(GLFWwindow* glfwWindow, int width, int height)
{
    (void)width;
    (void)height;
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mStatus |= Surface::Resized;
}

void glfw_framebuffer_size_callback(GLFWwindow* glfwWindow, int width, int height)
{
    (void)width;
    (void)height;
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mStatus |= Surface::Resized;
}

void glfw_char_callback(GLFWwindow* glfwWindow, unsigned int codepoint)
{
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mTextStream.push_back(codepoint);
}

void glfw_key_callback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mInput.keyboard.staged[(size_t)glfw_to_gvk_key(key)] = action == GLFW_PRESS || action == GLFW_REPEAT;
}

void glfw_cursor_pos_callback(GLFWwindow* glfwWindow, double xOffset, double yOffset)
{
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mInput.mouse.position.staged = { (float)xOffset, (float)yOffset };
}

void glfw_mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods)
{
    (void)mods;
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mInput.mouse.buttons.staged[(size_t)glfw_to_gvk_mouse_button(button)] = action == GLFW_PRESS || action == GLFW_REPEAT;
}

void glfw_scroll_callback(GLFWwindow* glfwWindow, double xOffset, double yOffset)
{
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    pSurfaceControlBlock->mInput.mouse.scroll.staged[0] += (float)xOffset;
    pSurfaceControlBlock->mInput.mouse.scroll.staged[1] += (float)yOffset;
}

void glfw_window_focus_callback(GLFWwindow* glfwWindow, int focused)
{
    auto pSurfaceControlBlock = (Surface::ControlBlock*)glfwGetWindowUserPointer(glfwWindow);
    assert(pSurfaceControlBlock);
    if (focused) {
        pSurfaceControlBlock->mStatus |= Surface::GainedFocus;
    } else {
        pSurfaceControlBlock->mStatus |= Surface::LostFocus;
    }
}

} // namespace system
} // namespace gvk
