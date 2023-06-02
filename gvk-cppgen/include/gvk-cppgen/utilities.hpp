
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

#include "gvk-cppgen/file-generator.hpp"
#include "gvk-string.hpp"
#include "gvk-xml.hpp"

#include <string>
#include <utility>

namespace gvk {
namespace cppgen {

bool is_static_const_value(const std::string& apiElementName);
bool is_strongly_typed_bitmask(const xml::Manifest& manifest, const std::string& apiElementName);
xml::Command append_return_result_parameter(xml::Command command);
std::string get_parameter_list(const std::vector<xml::Parameter>& parameters, bool types = true, bool names = true);
std::string get_extension_vendor(const std::string& str);
xml::Parameter create_parameter(const std::string& type, const std::string& name);
std::pair<xml::Parameter, xml::Parameter> get_array_parameters(const std::string& countName, const std::string& arrayName, const std::string& unqualifiedType);
void add_array_members_to_structure(const std::string& unqualifiedType, const std::string& countName, const std::string& arrayName, xml::Structure& structure);
std::set<std::string> get_inner_scope_compile_guards(const std::set<std::string>& outerScopeCompileGuards, std::set<std::string> innerScopeCompileGuards);
std::vector<string::Replacement> get_inner_scope_replacements(const std::vector<string::Replacement>& outerScopeReplacements, std::vector<string::Replacement> innerScopeReplacements);
void generate_noop_command_body(FileGenerator& file, const xml::Command& command);

void generate_pnext_switch(
    FileGenerator& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::string& caseProcessor,
    const std::string& defaultProcessor = std::string()
);

void generate_pnext_switch(
    FileGenerator& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::vector<std::string>& caseProcessor,
    const std::string& defaultProcessor = std::string()
);

void generate_object_type_switch(
    FileGenerator& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::string& caseProcessor,
    const std::string& defaultProcessor = std::string()
);

} // namespace cppgen
} // namespace gvk
