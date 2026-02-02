
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

#include <string>

namespace gvk {
namespace gui {

template <typename GuiInfoType>
class Window
{
public:
    class Manager;
    class Collection;

    virtual ~Window() = 0;

    virtual void reset()
    {
    }

    const std::string& get_name() const
    {
        return mName;
    }

    Window::Manager& get_window_manager()
    {
        return mWindowManager;
    }

protected:
    Window(Manager& windowManager, const std::string& name)
        : mWindowManager{ windowManager }
        , mName{ name }
    {
    }

    virtual void on_gui(GuiInfoType guiInfo) = 0;

private:
    Manager& mWindowManager;
    std::string mName;
    bool mOpen{ true };

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};

template <typename GuiInfoType>
Window<GuiInfoType>::~Window()
{
}

} // namespace gui
} // namespace gvk
