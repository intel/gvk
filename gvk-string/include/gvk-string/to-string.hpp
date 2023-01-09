
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

#include "gvk-string/printer.hpp"

#include <initializer_list>
#include <sstream>
#include <string>
#include <utility>

namespace gvk {

/**
Gets the std::string representation of a given object with a given configuration
@typename ObjectType The type of the object to get the std::string representation of
@param [in] obj The object to get the std::string representation of
@param [in] flags (optional = Printer::FlagBits::Default) Bitmask of Printer::FlagBits configuring printer output
@param [in] tabCount (optional = 0) The tab count to begin printing at
@param [in] tabSize (optional = 4) The tab size to use when printing tabs
@return The resulting std::string
*/
template <typename ObjectType>
inline std::string to_string(const ObjectType& obj, Printer::Flags flags = Printer::Default, int tabCount = 0, int tabSize = 4)
{
    std::stringstream strStrm;
    Printer printer(strStrm, flags, tabCount, tabSize);
    print(printer, obj);
    return strStrm.str();
}

/**
Gets the std::string representation of a specified value in hexadecimal
@typename T The type of value to get the hexadecimal string representation of
@param [in] value The value to get the hexadecimal string representation of
@return The resulting std::string
*/
template <typename T>
inline std::string to_hex_string(const T& value)
{
    char str[] = "0x0000000000000000";
    snprintf(str + 2, sizeof(str) - 2, "%llx", (long long unsigned int)value);
    return str;
}

/**
Gets a std::string of concatenated bitmask flag identifiers
@typename <FlagBitsType> The enum type of the specified bitmask
@typename <FlagsType> The type of the given flags
    @note Usually std::underlying_type_t<FlagBitsType>
@param [in] flags The flags value to check for each of the given identifiers
@param [in] flagIdentifiers A list of flags and their associated identifiers
@return The resulting std::string
*/
template <typename FlagBitsType, typename FlagsType>
inline std::string flags_to_string(FlagsType flags, std::initializer_list<std::pair<FlagBitsType, const char*>> flagIdentifiers)
{
    std::stringstream strStrm;
    for (const auto& flagIdentifier : flagIdentifiers) {
        if (flags & flagIdentifier.first) {
            assert(flagIdentifier.second);
            strStrm << flagIdentifier.second << "|";
        }
    }
    auto str = strStrm.str();
    if (!str.empty()) {
        str.pop_back();
    }
    return str;
}

} // namespace gvk
