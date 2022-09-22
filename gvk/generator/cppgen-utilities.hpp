
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

#include "gvk/xml/manifest.hpp"
#include "gvk/string.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>

namespace gvk {
namespace cppgen {

inline bool structure_requires_custom_implementation(const std::string& name)
{
    static const std::set<std::string> sStructures{
        // Win32
        "VkExportFenceWin32HandleInfoKHR",
        "VkExportMemoryWin32HandleInfoKHR",
        "VkExportMemoryWin32HandleInfoNV",
        "VkExportSemaphoreWin32HandleInfoKHR",
        "VkImportFenceWin32HandleInfoKHR",
        "VkImportMemoryWin32HandleInfoKHR",
        "VkImportMemoryWin32HandleInfoNV",
        "VkImportSemaphoreWin32HandleInfoKHR",

        // Video encode/decode
        "VkVideoDecodeH264DpbSlotInfoEXT",
        "VkVideoDecodeH264MvcEXT",
        "VkVideoDecodeH264PictureInfoEXT",
        "VkVideoDecodeH264SessionParametersAddInfoEXT",
        "VkVideoDecodeH265SessionParametersAddInfoEXT",
        "VkVideoDecodeH265DpbSlotInfoEXT",
        "VkVideoDecodeH265PictureInfoEXT",
        "VkVideoEncodeH264DpbSlotInfoEXT",
        "VkVideoEncodeH264NaluSliceEXT",
        "VkVideoEncodeH264ReferenceListsEXT",
        "VkVideoEncodeH264SessionParametersAddInfoEXT",
        "VkVideoEncodeH264VclFrameInfoEXT",
        "VkVideoEncodeH265DpbSlotInfoEXT",
        "VkVideoEncodeH265NaluSliceSegmentEXT",
        "VkVideoEncodeH265ReferenceListsEXT",
        "VkVideoEncodeH265SessionParametersAddInfoEXT",
        "VkVideoEncodeH265VclFrameInfoEXT",

        // Special case members
        "VkAccelerationStructureBuildGeometryInfoKHR",
        "VkAccelerationStructureVersionInfoKHR",
        "VkPipelineMultisampleStateCreateInfo",
        "VkShaderModuleCreateInfo",
        "VkTransformMatrixKHR",

        // Unions
        "VkAccelerationStructureGeometryDataKHR",
        "VkAccelerationStructureMotionInstanceDataNV",
        "VkClearColorValue",
        "VkClearValue",
        "VkDeviceOrHostAddressConstKHR",
        "VkDeviceOrHostAddressKHR",
        "VkPerformanceCounterResultKHR",
        "VkPerformanceValueDataINTEL",
        "VkPipelineExecutableStatisticValueKHR",
    };
    return sStructures.count(name) == 1;
}

inline bool structure_requires_custom_serialization(const std::string& name)
{
    static const std::set<std::string> sStructures{
        "VkAccelerationStructureInstanceKHR",
        "VkAccelerationStructureMatrixMotionInstanceNV",
        "VkAccelerationStructureSRTMotionInstanceNV",
        "VkSurfaceFullScreenExclusiveWin32InfoEXT",
        "VkWin32SurfaceCreateInfoKHR",
    };
    return structure_requires_custom_implementation(name) || sStructures.count(name) == 1;
}

inline bool static_const_values(const std::string& name)
{
    static const std::set<std::string> sEnums{
        "VkAccessFlagBits2",
        "VkFormatFeatureFlagBits2",
        "VkPipelineStageFlagBits2",
    };
    return sEnums.count(name) == 1;
}

class File final
    : public std::stringstream
{
public:
    inline File(const std::filesystem::path& filePath)
    {
        assert(!filePath.empty());
        if (filePath.extension() == ".h" ||
            filePath.extension() == ".hpp" ||
            filePath.extension() == ".inl") {
            mFilePath = std::filesystem::path(GVK_GENERATED_INCLUDE_PATH) / filePath;
        } else {
            mFilePath = std::filesystem::path(GVK_GENERATED_SOURCE_PATH) / filePath;
        }
        *this << R"(
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

// NOTE : This file contains generated code
)";
        if (filePath.extension() == ".h" ||
            filePath.extension() == ".hpp") {
            *this << R"(
#pragma once

#include "gvk/defines.hpp"
)";
        }
    }

    inline ~File()
    {
        assert(!mFilePath.empty());
        std::filesystem::create_directories(mFilePath.parent_path());
        auto content = str();
        std::ifstream file(mFilePath);
        if (content != std::string(std::istreambuf_iterator<char>(file), { })) {
            file.close();
            std::ofstream(mFilePath) << content;
        }
    }

private:
    std::filesystem::path mFilePath;
};

class Module final
{
public:
    inline Module(const std::string& name)
        : header(name + ".hpp")
        , source(name + ".cpp")
    {
        source << std::endl << "#include \"" << GVK_GENERATED_INCLUDE_PREFIX << name << ".hpp\"" << std::endl;
    }

    File header;
    File source;
};

class NamespaceGenerator final
{
public:
    inline NamespaceGenerator(std::ostream& ostream, const std::string& namespaces)
        : mOstream { ostream }
        , mNamespaces { string::split(namespaces, "::") }
    {
        for (auto itr = mNamespaces.begin(); itr != mNamespaces.end(); ++itr) {
            mOstream << "namespace " << *itr << " {" << std::endl;
        }
    }

    inline ~NamespaceGenerator()
    {
        for (auto ritr = mNamespaces.rbegin(); ritr != mNamespaces.rend(); ++ritr) {
            mOstream << "} // namespace " << *ritr << std::endl;
        }
    }

private:
    std::ostream& mOstream;
    std::vector<std::string> mNamespaces;
};

class CompileGuardGenerator final
{
public:
    inline CompileGuardGenerator(std::ostream& ostream, const std::set<std::string>& compileGuards)
        : mOstream { ostream }
        , mCompileGuards { compileGuards }
    {
        for (auto itr = mCompileGuards.begin(); itr != mCompileGuards.end(); ++itr) {
            mOstream << "#ifdef " << *itr << std::endl;
        }
    }

    inline ~CompileGuardGenerator()
    {
        for (auto ritr = mCompileGuards.rbegin(); ritr != mCompileGuards.rend(); ++ritr) {
            mOstream << "#endif // " << *ritr << std::endl;
        }
    }

private:
    std::ostream& mOstream;
    std::set<std::string> mCompileGuards;
};

std::set<std::string> get_inner_scope_compile_guards(
    const std::set<std::string>& outerScopeCompileGuards,
    std::set<std::string> innerScopeCompileGuards
)
{
    std::erase_if(innerScopeCompileGuards, [&](const auto& compileGuard) { return outerScopeCompileGuards.count(compileGuard) == 1; });
    return innerScopeCompileGuards;
}

inline std::vector<string::Replacement> get_inner_scope_replacements(
    const std::vector<string::Replacement>& outerScopeReplacements,
    std::vector<string::Replacement> innerScopeReplacements
)
{
    std::set<std::string> innerScopeKeys;
    for (const auto& innerScopeReplacement : innerScopeReplacements) {
        innerScopeKeys.insert(innerScopeReplacement.first);
    }
    for (const auto& outerScopeReplacement : outerScopeReplacements) {
        if (!innerScopeKeys.count(outerScopeReplacement.first)) {
            innerScopeReplacements.push_back(outerScopeReplacement);
        }
    }
    return innerScopeReplacements;
}

inline void generate_pnext_switch(
    File& file,
    const xml::Manifest& manifest,
    const std::string& indentation,
    const std::string& evaluation,
    const std::string& caseProcessor,
    const std::string& defaultProcessor
)
{
    file << indentation << "switch (" << evaluation << ") {" << std::endl;
    for (const auto& structureItr : manifest.structures) {
        const auto& structure = structureItr.second;
        if (structure.alias.empty() && !structure.vkStructureType.empty()) {
            CompileGuardGenerator compileGuardGenerator(file, structure.compileGuards);
            file << indentation << "case " << structure.vkStructureType << ": {" << std::endl;
            std::vector<string::Replacement> replacements{
                { "{structureType}", structure.name },
                { "{sType}", structure.vkStructureType },
            };
            file << indentation << "    " << string::replace(caseProcessor, replacements) << std::endl;
            file << indentation << "} break;" << std::endl;
        }
    }
    file << indentation << "default: {" << std::endl;
    file << indentation << "    " << defaultProcessor << std::endl;
    file << indentation << "}" << std::endl;
    file << indentation << "}" << std::endl;
}

inline std::string get_command_args(const xml::Command& command, bool types = true, bool names = true)
{
    std::stringstream strStrm;
    int count = 0;
    for (const auto& parameter : command.parameters) {
        if (count++) {
            strStrm << ", ";
        }
        if (types) {
            strStrm << parameter.type;
        }
        if (types && names) {
            strStrm << " ";
        }
        if (names) {
            strStrm << parameter.name;
        }
    }
    return strStrm.str();
}

inline bool is_strongly_typed_bitmask(const xml::Manifest& manifest, const std::string& typeName)
{
    auto flagBitsTypeName = string::replace(typeName, "Flags", "FlagBits");
    const auto& enumerationItr = manifest.enumerations.find(flagBitsTypeName);
    if (enumerationItr != manifest.enumerations.end()) {
        const auto& enumeration = enumerationItr->second;
        return enumeration.isBitmask && !enumeration.enumerators.empty() && !static_const_values(enumeration.name);
    }
    return false;
}

class StructureMemberGenerator
{
public:
    virtual ~StructureMemberGenerator() = 0;

    std::string generate(const xml::Manifest& manifest, const xml::Parameter& member) const
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

protected:
    inline virtual std::string generate_pnext_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_void_pointer_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_function_pointer_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_dynamic_handle_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_dynamic_structure_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_dynamic_enumeration_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_dynamic_string_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_dynamic_string_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_dynamic_primitive_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_handle_pointer_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_structure_pointer_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_enumeration_pointer_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_primitive_pointer_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_static_handle_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_static_structure_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_static_enumeration_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_static_string_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_static_primitive_array_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_handle_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_structure_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_enumeration_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_flags_processor() const
    {
        return std::string();
    }

    inline virtual std::string generate_primitive_processor() const
    {
        return std::string();
    }
};

inline StructureMemberGenerator::~StructureMemberGenerator()
{
}

#if 0
class BoilerPlateStructureMemberGenerator final
    : public StructureMemberGenerator
{
protected:
    std::string generate_pnext_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_void_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_function_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_dynamic_handle_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_dynamic_structure_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_dynamic_enumeration_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_dynamic_string_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_dynamic_string_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_dynamic_primitive_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_handle_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_structure_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_enumeration_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_primitive_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_static_handle_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_static_structure_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_static_enumeration_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_static_string_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_static_primitive_array_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_handle_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_structure_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_enumeration_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_flags_processor() const override final
    {
        return "// {memberName}";
    }

    std::string generate_primitive_processor() const override final
    {
        return "// {memberName}";
    }
};
#endif

} // namespace cppgen
} // namespace gvk
