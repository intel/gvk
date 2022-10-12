
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

#include <algorithm>
#include <ios>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace gvk {
namespace string {

/**
White space characters
*/
static constexpr char WhiteSpaceCharacters[] = " \f\n\r\t\v";

/**
Alias for std::pair<string::string, string::string> used for replace() operations
*/
using Replacement = std::pair<std::string, std::string>;

/**
Gets a value indicating whether or not a given string contains another given string
@param [in] str The string to search
@param [in] find The string to find
@return Whether or not the given string contains the given find string
*/
bool contains(const std::string& str, const std::string& find);

/**
Gets a value indicating whether or not a given string starts with a given find string
@param [in] str The string to search
@param [in] find The string to find
@return Whether or not the given string starts with the given find string
*/
bool starts_with(const std::string& str, const std::string& find);

/**
Gets a value indicating whether or not a given string ends with a given find string
@param [in] str The string to search
@param [in] find The string to find
@return Whether or not the given string ends with the given find string
*/
bool ends_with(const std::string& str, const std::string& find);

/**
Gets a copy of a string with all occurences of a given substring replaced with another
@param [in] str The source string
@param [in] find The string to find and replace in the source string
@param [in] replacement The string to replace occurences of the find string with
@param [in] recursive (optional = false) Whether or not to recursively replace occurences of the find string
@return The resulting std::string
*/
std::string replace(const std::string& str, const std::string& find, const std::string& replacement, bool recursive = false);

/**
Gets a copy of a string with a all occurences of a given collection of substrings replaced with a paired replacement
@param [in] str The source string
@param [in] replacements The collection of find and replace pairs
@return The resulting std::string
*/
std::string replace(const std::string& str, const std::vector<Replacement>& replacements);

/**
Gets a copy of a string with all occurences of a given substring removed
@param [in] str The source string
@param [in] find The string to find and remove from the source string
@param [in] recursive (optional = false) Whether or not to recursively remove occurences of the find string
@return The resulting std::string
*/
std::string remove(const std::string& str, const std::string& find, bool recurisve = false);

/**
Gets a copy of a string with a all occurences of a given collection of substrings removed
@param [in] str The source string
@param [in] finds The collection of strings to remove
@return The resulting std::string
*/
std::string remove(const std::string& str, const std::vector<std::string>& finds);

/**
Gets a copy of a string with all repetitive occurences of a given substring reduced to single occurences
@param [in] str The source string
@param [in] find The string to find sequences of and reduce
@return The resulting std::string
*/
std::string reduce_sequence(const std::string& str, const std::string& find);

/**
Gets a copy of a string containing a path with back slashes replaced with forward slashes and slash sequences reduced
@param [in] path The path to scrub
@return The resulting std::string
*/
std::string scrub_path(const std::string& path);

/**
Gets a value indicating whether or not a specified char is a numeric character
@param [in] c The char to check
@return Whether or not the specified char is a numeric character
*/
bool is_number(char c);

/**
Gets a value indicating whether or not a given string is all numeric characters
@param [in] str The string to check
@return Whether or not the given string is all numeric characters
*/
bool is_number(const std::string& str);

/**
Gets a value indicating whether or not a specified char is a whitespace character
@param [in] c The char to check
@return Whether or not the specified char is a whitespace character
*/
bool is_whitespace(char c);

/**
Gets a value indicating whether or not a given string is all whitespace characters
@param [in] str The string to check
@return Whether or not the given string is all whitespace characters
*/
bool is_whitespace(const std::string& str);

/**
Gets a copy of string with all leading whitespace characters removed
@param [in] str The string to remove leading whitespace characters from
@return The resulting std::string
*/
inline std::string trim_leading_whitespace(const std::string& str);

/**
Gets a copy of string with all trailing whitespace characters removed
@param [in] str The string to remove trailing whitespace characters from
@return The resulting std::string
*/
std::string trim_trailing_whitespace(const std::string& str);

/**
Gets a copy of string with all leading and trailing whitespace characters removed
@param [in] str The string to remove leading and trailing whitespace characters from
@return The resulting std::string
*/
std::string trim_whitespace(const std::string& str);

/**
Gets a value indicating whether or not a specified char is an upper case character
@param [in] c The char to check
@return Whether or not the specified char is an upper case character
*/
bool is_upper(char c);

/**
Gets a value indicating whether or not a given string is all upper case characters
@param [in] str The string to check
@return Whether or not the given string is all upper case characters
*/
bool is_upper(const std::string& str);

/**
Gets the upper case equivalent of a specififed character
@param [in] c The character to get the upper case equivalent of
@retuern The upper case equivalent of the specified character
*/
char to_upper(char c);

/**
Gets a copy of a given string with all characters converted to their upper case equivalents
@param [in] str The string to convert to upper case
@return The resulting std::string
*/
std::string to_upper(const std::string& str);

/**
Gets a value indicating whether or not a specified char is a lower case character
@param [in] c The char to check
@return Whether or not the specified char is a lower case character
*/
bool is_lower(char c);

/**
Gets a value indicating whether or not a given string is all lower case characters
@param [in] str The string to check
@return Whether or not the given string is all lower case characters
*/
bool is_lower(const std::string& str);

/**
Gets the lower case equivalent of a specififed character
@param [in] c The character to get the lower case equivalent of
@retuern The lower case equivalent of the specified character
*/
char to_lower(char c);

/**
Gets a copy of a given string with all characters converted to their lower case equivalents
@param [in] str The string to convert to lower case
@return The resulting std::string
*/
std::string to_lower(const std::string& str);

/**
Gets a std::vector<std::string> populated with substrings of a given string using a given delimiter
@param [in] str The string to split
@param [in] delimiter The delimiter to search for
@return A std::vector<std::string> populated with split tokens
*/
std::vector<std::string> split(const std::string& str, const std::string& delimiter);

/**
Gets a std::vector<std::string> populated with substrings of a given string using a '_' as a delimiter
@param [in] str The string to split
@return A std::vector<std::string> populated with split tokens
*/
std::vector<std::string> split_snake_case(const std::string& str);

/**
Gets a std::vector<std::string> populated with substrings of a given string split on CamelCase
@param [in] str The string to split
@return A std::vector<std::string> populated with split tokens
*/
std::vector<std::string> split_camel_case(const std::string& str);

/**
Gets a copy of a string with all occurences of "vk", "Vk", and "VK_" removed
@param [in] str The source string
@return The resulting std::string
*/
std::string strip_vk(const std::string& str);

/**
Converts a given string to a number of a specified type
@typename T The type of number to convert the given string to
@param [in] str The string to convert into a number
    @NOTE : The given string may be prepended with "0x" (case insensitive) to indicate that the provided value is hexidecimal
@return The number converted from the given string
*/
template <typename T>
inline T to_number(const std::string& str)
{
    std::stringstream strStrm;
    if (2 < str.size() && str[0] == '0' && to_lower(str[1]) == 'x') {
        strStrm << std::hex;
    }
    strStrm << str;
    T number{ };
    strStrm >> number;
    return number;
}

} // namespace string
} // namespace gvk
