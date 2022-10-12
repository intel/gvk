
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

#include "gvk/string.hpp"

namespace gvk {
namespace string {

bool contains(const std::string& str, const std::string& find)
{
    return str.find(find.c_str()) != std::string::npos;
}

bool starts_with(const std::string& str, const std::string& find)
{
    return find.size() <= str.size() && str.substr(0, find.size()) == find;
}

bool ends_with(const std::string& str, const std::string& find)
{
    return find.size() <= str.size() && str.substr(str.size() - find.size(), find.size()) == find;
}

std::string replace(const std::string& str, const std::string& find, const std::string& replacement, bool recursive)
{
    auto result = str;
    if (!find.empty() && find != replacement) {
        auto index = result.find(find);
        while (index != std::string::npos) {
            result.replace(index, find.size(), replacement);
            index += recursive ? 0 : replacement.size();
            index = result.find(find, index);
        }
    }
    return result;
}

std::string replace(const std::string& str, const std::vector<Replacement>& replacements)
{
    auto result = str;
    auto sortedReplacements = replacements;
    std::sort(
        sortedReplacements.begin(),
        sortedReplacements.end(),
        [](const auto& lhs, const auto& rhs)
        {
            return lhs.first.length() > rhs.first.length();
        }
    );
    for (const auto& replacement : sortedReplacements) {
        result = replace(result, replacement.first, replacement.second);
    }
    return result;
}

std::string remove(const std::string& str, const std::string& find, bool recurisve)
{
    return replace(str, find, std::string(), recurisve);
}

std::string remove(const std::string& str, const std::vector<std::string>& finds)
{
    auto result = str;
    for (const auto& find : finds) {
        result = remove(result, find);
    }
    return result;
}

std::string reduce_sequence(const std::string& str, const std::string& find)
{
    return replace(str, find + find, find, true);
}

std::string scrub_path(const std::string& path)
{
    return reduce_sequence(replace(path, "\\", "/"), "/");
}

bool is_number(char c)
{
    return std::isdigit((int)c);
}

bool is_number(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), [](char c) { return is_number(c); });
}

bool is_whitespace(char c)
{
    return std::isspace((int)c);
}

bool is_whitespace(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), [](char c) { return is_whitespace(c); });
}

std::string trim_leading_whitespace(const std::string& str)
{
    auto offset = str.find_first_not_of(WhiteSpaceCharacters);
    return offset == std::string::npos ? std::string() : str.substr(offset);
}

std::string trim_trailing_whitespace(const std::string& str)
{
    auto offset = str.find_last_not_of(WhiteSpaceCharacters);
    return offset == std::string::npos ? std::string() : str.substr(0, offset + 1);
}

std::string trim_whitespace(const std::string& str)
{
    return trim_leading_whitespace(trim_trailing_whitespace(str));
}

bool is_upper(char c)
{
    return std::isupper((int)c);
}

bool is_upper(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), [](char c) { return is_upper(c); });
}

char to_upper(char c)
{
    return (char)std::toupper((int)c);
}

std::string to_upper(const std::string& str)
{
    auto result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](char c) { return to_upper(c); });
    return result;
}

bool is_lower(char c)
{
    return std::islower((int)c);
}

bool is_lower(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), [](char c) { return is_lower(c); });
}

char to_lower(char c)
{
    return (char)std::tolower((int)c);
}

std::string to_lower(const std::string& str)
{
    auto result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](char c) { return to_lower(c); });
    return result;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> tokens;
    if (!str.empty() && !delimiter.empty()) {
        size_t index = 0;
        size_t offset = 0;
        while ((index = str.find(delimiter.c_str(), offset)) != std::string::npos) {
            if (index - offset > 0) {
                tokens.push_back(std::string(str.substr(offset, index - offset)));
            }
            offset = index + delimiter.size();
        }
        if (offset < str.length()) {
            tokens.push_back(std::string(str.substr(offset, str.length() - offset)));
        }
    }
    return tokens;
}

std::vector<std::string> split_snake_case(const std::string& str)
{
    return split(str, "_");
}

std::vector<std::string> split_camel_case(const std::string& str)
{
    std::vector<std::string> tokens;
    if (!str.empty()) {
        auto begin = std::find_if_not(str.begin(), str.end(), [](char c) { return c == '_'; });
        while (begin != str.end()) {
            auto end = std::find_if(std::next(begin), str.end(), [](char c) { return c == '_' || is_upper(c); });
            tokens.emplace_back(begin, end);
            begin = std::find_if_not(end, str.end(), [](char c) { return c == '_'; });
        }
        for (size_t leadToken_i = 0; leadToken_i < tokens.size(); ++leadToken_i) {
            auto& leadToken = tokens[leadToken_i];
            if (leadToken.size() == 1) {
                size_t followToken_i = leadToken_i + 1;
                while (followToken_i < tokens.size() && tokens[followToken_i].size() == 1) {
                    leadToken += tokens[followToken_i];
                    tokens.erase(tokens.begin() + followToken_i);
                }
            }
        }
    }
    return tokens;
}

std::string strip_vk(const std::string& str)
{
    return remove(remove(remove(str, "vk"), "Vk"), "VK_");
}

} // namespace string
} // namespace gvk
