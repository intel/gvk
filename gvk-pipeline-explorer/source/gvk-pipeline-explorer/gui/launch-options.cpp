
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

#include "gvk-pipeline-explorer/gui/launch-options.hpp"

#include <cassert>

namespace gvk {
namespace pipeline_explorer {
namespace gui {

void LaunchOptions::im_gui_settings_write(const LaunchOptions& launchOptions, ImGuiContext* pCtx, ImGuiSettingsHandler* pSettingsHandler, ImGuiTextBuffer* pTextBuffer)
{
    (void)pCtx;
    (void)pSettingsHandler;
    assert(pTextBuffer);
    auto getCStr = [](const std::string& str) { return !str.empty() ? str.c_str() : "nullptr"; };
    pTextBuffer->appendf("exe=%s\n", getCStr(launchOptions.exe));
    pTextBuffer->appendf("target=%s\n", getCStr(launchOptions.target));
    pTextBuffer->appendf("args=%s\n", getCStr(launchOptions.args));
    pTextBuffer->appendf("directory=%s\n", getCStr(launchOptions.directory));
    pTextBuffer->appendf("workspace=%s\n", getCStr(launchOptions.workspace));
    pTextBuffer->appendf("logPath=%s\n", getCStr(launchOptions.logPath));
    pTextBuffer->appendf("waitForDebugger=%d\n", (int)launchOptions.waitForDebugger);
    pTextBuffer->appendf("autoDirectory=%d\n", (int)launchOptions.autoDirectory);
    pTextBuffer->appendf("autoWorkspace=%d\n", (int)launchOptions.autoWorkspace);
    pTextBuffer->appendf("autoLogPath=%d\n", (int)launchOptions.autoLogPath);
    pTextBuffer->appendf("logToStdOut=%d\n", (int)launchOptions.logToStdOut);
    pTextBuffer->appendf("logToFile=%d\n", (int)launchOptions.logToFile);
    pTextBuffer->appendf("loaderDebug=%d\n", (int)launchOptions.loaderDebug);
    pTextBuffer->appendf("clearStdOut=%d\n", (int)launchOptions.clearStdOut);
    pTextBuffer->appendf("uniqueHandles=%d\n", (int)launchOptions.uniqueHandles);
    pTextBuffer->appendf("\n");
}

} // namespace gui
} // namespace pipeline_explorer
} // namespace gvk
