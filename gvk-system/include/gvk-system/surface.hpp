
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
#include "gvk-reference.hpp"

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#include <array>
#include <unordered_map>
#include <vector>

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

        uint32_t flags{ Decorated | Resizable | Visible };
        const char* pTitle{ nullptr };
        std::array<int32_t, 2> position{ };
        std::array<int32_t, 2> extent{ 1280, 720 };
        CursorMode cursorMode{ CursorMode::Visible };
    };

    struct PlatformInfo
    {
#if defined(__linux__)
        Display* x11Display{ nullptr };
        Window x11Window{ };
#endif
#if defined(_WIN32) || defined(_WIN64)
        HWND hwnd{ NULL };
#endif
    };

    static int32_t create(const CreateInfo* pCreateInfo, Surface* pSurface);
    static void update();
    void get_window_position(int32_t* pX, int32_t* pY) const;
    void get_window_extent(int32_t* pWidth, int32_t* pHeight) const;
    void set_window_extent(const std::array<int32_t, 2>& extent);

    template <typename T>
    const T& get() const
    {
        if constexpr (std::is_same_v<T, Input>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return mReference->mInput; }
        if constexpr (std::is_same_v<T, StatusFlags>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return mReference->mStatus; }
        if constexpr (std::is_same_v<T, TextStream>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return mReference->mTextStream; }
        if constexpr (std::is_same_v<T, std::string>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return get_title(); }
        if constexpr (std::is_same_v<T, CursorMode>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return get_cursor_mode(); }
        if constexpr (std::is_same_v<T, PlatformInfo>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return get_platform_info(); }
    }

    template <typename T>
    void set(const T& value)
    {
        if constexpr (std::is_same_v<T, std::string>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return set_title(value); }
        if constexpr (std::is_same_v<T, CursorMode>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return set_cursor_mode(value); }
        if constexpr (std::is_same_v<T, CursorType>) { assert(mReference && "Attempting to dereference nullref gvk::system::Surface"); return set_cursor_type(value); }
    }

    using TextStream = std::vector<uint32_t>;

    class ControlBlock final
    {
    public:
        ControlBlock();
        ~ControlBlock();
        Input mInput;
        std::string mTitle;
        StatusFlags mStatus{ };
        TextStream mTextStream;
        CursorMode mCursorMode{ };
        void* mpWindowHandle{ nullptr };
        PlatformInfo mPlatformInfo{ };
    private:
        ControlBlock(const ControlBlock&) = delete;
        ControlBlock& operator=(const ControlBlock&) = delete;
    };

private:
    const std::string& get_title() const;
    void set_title(const std::string& title);
    const CursorMode& get_cursor_mode() const;
    void set_cursor_mode(CursorMode cursorMode);
    void set_cursor_type(CursorType cursorType);
    const PlatformInfo& get_platform_info() const;
    gvk_reference_type(Surface)
};

} // namespace system
} // namespace gvk
