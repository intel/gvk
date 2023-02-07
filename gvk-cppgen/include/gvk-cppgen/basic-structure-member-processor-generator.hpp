
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

#include "gvk-xml.hpp"

#include <string>

namespace gvk {
namespace cppgen {

class BasicStructureMemberProcessorGenerator
{
public:
    virtual ~BasicStructureMemberProcessorGenerator() = 0;
    std::string generate(const xml::Manifest& manifest, const xml::Parameter& member) const;

protected:
    virtual std::string generate_pnext_processor() const;
    virtual std::string generate_void_pointer_processor() const;
    virtual std::string generate_function_pointer_processor() const;
    virtual std::string generate_dynamic_handle_array_processor() const;
    virtual std::string generate_dynamic_structure_array_processor() const;
    virtual std::string generate_dynamic_enumeration_array_processor() const;
    virtual std::string generate_dynamic_string_processor() const;
    virtual std::string generate_dynamic_string_array_processor() const;
    virtual std::string generate_dynamic_primitive_array_processor() const;
    virtual std::string generate_handle_pointer_processor() const;
    virtual std::string generate_structure_pointer_processor() const;
    virtual std::string generate_enumeration_pointer_processor() const;
    virtual std::string generate_primitive_pointer_processor() const;
    virtual std::string generate_static_handle_array_processor() const;
    virtual std::string generate_static_structure_array_processor() const;
    virtual std::string generate_static_enumeration_array_processor() const;
    virtual std::string generate_static_string_processor() const;
    virtual std::string generate_static_primitive_array_processor() const;
    virtual std::string generate_handle_processor() const;
    virtual std::string generate_structure_processor() const;
    virtual std::string generate_enumeration_processor() const;
    virtual std::string generate_flags_processor() const;
    virtual std::string generate_primitive_processor() const;

    xml::Parameter mMember;
};

#if 0
class BoilerPlateStructureMemberProcessorGenerator final
    : public BasicStructureMemberProcessorGenerator
{
protected:
    inline std::string generate_pnext_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_void_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_function_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_dynamic_handle_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_dynamic_structure_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_dynamic_enumeration_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_dynamic_string_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_dynamic_string_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_dynamic_primitive_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_handle_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_structure_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_enumeration_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_primitive_pointer_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_static_handle_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_static_structure_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_static_enumeration_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_static_string_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_static_primitive_array_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_handle_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_structure_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_enumeration_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_flags_processor() const override final
    {
        return "// {memberName}";
    }

    inline std::string generate_primitive_processor() const override final
    {
        return "// {memberName}";
    }
};
#endif

} // namespace cppgen
} // namespace gvk
