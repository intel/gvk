
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

#include "gvk/cppgen/file-generator.hpp"
#include "gvk/xml/command.hpp"
#include "gvk/xml/manifest.hpp"
#include "gvk/string.hpp"

#include <string>

namespace gvk {
namespace cppgen {

bool is_static_const_value(const std::string& apiElementName);
bool is_strongly_typed_bitmask(const xml::Manifest& manifest, const std::string& apiElementName);
bool structure_requires_custom_implementation(const std::string& name);
bool structure_requires_custom_serialization(const std::string& name);
std::string get_command_args(const xml::Command& command, bool types = true, bool names = true);

std::set<std::string> get_inner_scope_compile_guards(
    const std::set<std::string>& outerScopeCompileGuards,
    std::set<std::string> innerScopeCompileGuards
);

std::vector<string::Replacement> get_inner_scope_replacements(
    const std::vector<string::Replacement>& outerScopeReplacements,
    std::vector<string::Replacement> innerScopeReplacements
);

void generate_pnext_switch(
    FileGenerator& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::string& caseProcessor,
    const std::string& defaultProcessor
);

} // namespace cppgen
} // namespace gvk
