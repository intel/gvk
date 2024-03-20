
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


#include "gvk-sample-utilities.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class LogEntry final
{
public:
    void reset()
    {
        mLines.clear();
    }

    bool add_line(const std::string& line)
    {
        if (!line.empty() && !gvk::string::is_whitespace(line)) {
            auto threadFrameEntry =
                mLines.size() == 1 &&
                gvk::string::contains(mLines[0], "Thread ") &&
                gvk::string::contains(mLines[0], ", Frame ") &&
                gvk::string::contains(mLines[0], ":");
            auto colorEntry = line.size() == 1 && line == ":";
            if (mLines.empty() ||
                gvk::string::is_whitespace(line[0]) ||
                threadFrameEntry ||
                colorEntry
            ) {
                mLines.push_back(line);
                return true;
            }
        }
        return false;
    }

    std::string to_string() const
    {
        std::stringstream strStrm;
        for (const auto& line : mLines) {
            strStrm << line << std::endl;
        }
        auto str = strStrm.str();
        while (!str.empty() && gvk::string::is_whitespace(str.back())) {
            str = str.substr(0, str.size() - 1);
        }
        return str;
    }

private:
    std::vector<std::string> mLines;
};

using CmdLine = std::map<std::string, std::string>;
static CmdLine get_cmd_line(int argc, const char* ppArgv[], CmdLine cmdLine = { })
{
    for (int i = 0; i < argc; ++i) {
        auto itr = cmdLine.insert({ ppArgv[i], { } }).first;
        if (gvk::string::starts_with(itr->first, "-")) {
            if (i < argc - 1) {
                itr->second = ppArgv[i + 1];
                ++i;
            }
        }
    }
    return cmdLine;
}

static std::vector<std::string> parse_log(const std::filesystem::path& filePath)
{
    std::vector<std::string> logEntries;
    std::ifstream file(filePath);
    if (file.is_open()) {
        LogEntry logEntry;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                if (!logEntry.add_line(line)) {
                    auto str = logEntry.to_string();
                    if (!str.empty()) {
                        logEntries.push_back(str);
                    }
                    logEntry.reset();
                    logEntry.add_line(line);
                }
            }
        }
    }
    return logEntries;
}

void output_function_counts(const std::vector<std::string>& logEntries)
{
    std::map<std::string, uint32_t> functionCounts;
    for (const auto& logEntry : logEntries) {
        if (gvk::string::contains(logEntry, "Thread") && gvk::string::contains(logEntry, "Frame")) {
            auto functionNameBegin = logEntry.find_first_of(':') + 3;
            auto functionNameEnd = logEntry.find_first_of('(');
            auto functionNameLength = functionNameEnd - functionNameBegin;
            auto functionName = logEntry.substr(functionNameBegin, functionNameLength);
            ++functionCounts[functionName];
        }
    }
    for (const auto& functionCountItr : functionCounts) {
        std::cout << functionCountItr.first << " : " << functionCountItr.second << std::endl;
    }
}

int main(int argc, const char* ppArgv[])
{
    auto cmdLine = get_cmd_line(argc, ppArgv, { {"-x", GVK_XML_FILE_PATH} });
    if (!cmdLine.count("-f") || cmdLine.count("-h")) {
        std::cout << "-h : Output this help text" << std::endl;
        std::cout << "-f : Filepath to the log to parse; required" << std::endl;
        std::cout << "-x : vk.xml filepath; if not provided GVK_XML_FILE_PATH will be used" << std::endl;
    } else {
        output_function_counts(parse_log(cmdLine["-f"]));
    }
}
