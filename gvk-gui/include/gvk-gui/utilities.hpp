
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

#include "imgui.h"
#include "imgui_stdlib.h"

namespace GvkGui {

class ScopeID final
{
public:
    ScopeID(const char* str_id);
    ScopeID(const char* str_id_begin, const char* str_id_end);
    ScopeID(const void* ptr_id);
    ScopeID(int int_id);
    ~ScopeID();

private:
    ScopeID(const ScopeID&) = delete;
    ScopeID& operator=(const ScopeID&) = delete;
};

class ScopeIndent final
{
public:
    ScopeIndent();
    ~ScopeIndent();

private:
    ScopeIndent(const ScopeIndent&) = delete;
    ScopeIndent& operator=(const ScopeIndent&) = delete;
};

bool InputText(const char* label, std::string* str);
bool InputPath(const char* label, std::string* str);

} // namespace GvkGui
