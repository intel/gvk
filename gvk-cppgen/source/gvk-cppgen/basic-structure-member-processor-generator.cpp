
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

#include "gvk-cppgen/basic-structure-member-processor-generator.hpp"
#include "gvk-cppgen/utilities.hpp"
#include "gvk-string/utilities.hpp"

namespace gvk {
namespace cppgen {

BasicStructureMemberProcessorGenerator::~BasicStructureMemberProcessorGenerator()
{
}

std::string BasicStructureMemberProcessorGenerator::generate(const xml::Manifest& manifest, const xml::Parameter& member) const
{
    std::string source;
    if (member.name == "pNext") {
        source = generate_pnext_processor();
    } else if (member.flags & xml::Pointer) {
        if (member.flags & xml::Void) {
            source = generate_void_pointer_processor();
        } else if (member.flags & xml::Function) {
            source = generate_function_pointer_processor();
        } else if (member.flags & xml::Array) {
            if (manifest.handles.count(member.unqualifiedType)) {
                source = generate_dynamic_handle_array_processor();
            } else if (manifest.structures.count(member.unqualifiedType)) {
                source = generate_dynamic_structure_array_processor();
            } else if (manifest.enumerations.count(member.unqualifiedType)) {
                source = generate_dynamic_enumeration_array_processor();
            } else if (member.type == "const char*") {
                source = generate_dynamic_string_processor();
            } else if (member.type == "const char* const*") {
                source = generate_dynamic_string_array_processor();
            } else {
                source = generate_dynamic_primitive_array_processor();
            }
        } else {
            if (manifest.handles.count(member.unqualifiedType)) {
                source = generate_handle_pointer_processor();
            } else if (manifest.structures.count(member.unqualifiedType)) {
                source = generate_structure_pointer_processor();
            } else if (manifest.enumerations.count(member.unqualifiedType)) {
                source = generate_enumeration_pointer_processor();
            } else {
                source = generate_primitive_pointer_processor();
            }
        }
    } else {
        if (member.flags & xml::Array) {
            if (manifest.handles.count(member.unqualifiedType)) {
                source = generate_static_handle_array_processor();
            } else if (manifest.structures.count(member.unqualifiedType)) {
                source = generate_static_structure_array_processor();
            } else if (manifest.enumerations.count(member.unqualifiedType)) {
                source = generate_static_enumeration_array_processor();
            } else if (member.type == "char") {
                source = generate_static_string_processor();
            } else {
                source = generate_static_primitive_array_processor();
            }
        } else {
            if (manifest.handles.count(member.unqualifiedType)) {
                source = generate_handle_processor();
            } else if (manifest.structures.count(member.unqualifiedType)) {
                source = generate_structure_processor();
            } else if (manifest.enumerations.count(member.unqualifiedType)) {
                source = generate_enumeration_processor();
            } else if (is_strongly_typed_bitmask(manifest, member.type)) {
                source = generate_flags_processor();
            } else {
                source = generate_primitive_processor();
            }
        }
    }
    return string::replace(source, {
        { "{memberType}", member.type },
        { "{memberName}", member.name },
        { "{memberUnqualifiedType}", member.unqualifiedType },
        { "{memberLength}", string::remove(string::remove(member.length, "["), "]") },
        { "{memberFlagBitsType}", string::replace(member.type, "Flags", "FlagBits") },
    });
}

std::string BasicStructureMemberProcessorGenerator::generate_pnext_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_void_pointer_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_function_pointer_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_dynamic_handle_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_dynamic_structure_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_dynamic_enumeration_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_dynamic_string_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_dynamic_string_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_dynamic_primitive_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_handle_pointer_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_structure_pointer_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_enumeration_pointer_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_primitive_pointer_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_static_handle_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_static_structure_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_static_enumeration_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_static_string_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_static_primitive_array_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_handle_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_structure_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_enumeration_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_flags_processor() const
{
    return std::string();
}

std::string BasicStructureMemberProcessorGenerator::generate_primitive_processor() const
{
    return std::string();
}

} // namespace cppgen
} // namespace gvk
