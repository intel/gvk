
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

#include "gvk-string.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

class LogEntry final
{
public:
    void reset()
    {
        mLines.clear();
        mString.clear();
    }

    bool add_line(const std::string& line)
    {
        mString.clear();
        if (!line.empty() && !gvk::string::is_whitespace(line)) {
            auto threadFrameEntry =
                mLines.size() == 1 &&
                gvk::string::contains(mLines[0], "Thread ") &&
                gvk::string::contains(mLines[0], ", Frame ") &&
                gvk::string::contains(mLines[0], ":");
            auto colorEntry = line.size() == 1 && line == ":";
            if (mLines.empty() || gvk::string::is_whitespace(line[0]) || threadFrameEntry || colorEntry) {
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

    const std::string& to_string()
    {
        if (mString.empty()) {
            mString = const_cast<const LogEntry*>(this)->to_string();
        }
        return mString;
    }

private:
    std::vector<std::string> mLines;
    std::string mString;
};

std::vector<std::string> parse_log(const std::filesystem::path& filePath)
{
    std::vector<std::string> logEntries;
    std::ifstream file(filePath, std::ifstream::ate);
    auto size = file.tellg();
    file.seekg(0);
    if (file.is_open()) {
        LogEntry logEntry;
        std::string logEntryStr;
        std::string line;
        while (std::getline(file, line)) {
            auto progress = file.tellg();
            if (!(progress % 100000)) {
                std::cout << (float)progress / (float)size * 100.0f << '%' << std::endl;
            }
            if (!line.empty()) {
                if (!logEntry.add_line(line)) {
                    if (!logEntry.to_string().empty()) {
                        logEntries.push_back(logEntry.to_string());
                    }
                    logEntry.reset();
                    logEntry.add_line(line);
                }
            }
        }
        if (!logEntry.to_string().empty()) {
            logEntries.push_back(logEntry.to_string());
        }
    }
    std::cout << "100.00%" << std::endl;
    return logEntries;
}

bool log_entry_contains(const std::string& logEntry, const std::set<std::string>& filters)
{
    for (const auto& filter : filters) {
        if (gvk::string::contains(logEntry, filter)) {
            return true;
        }
    }
    return false;
}

bool log_entry_unique_or_unfiltered(const std::string& logEntry, const std::set<std::string>& uniques, std::set<std::string>& encounteredUniques)
{
    bool uniqueOrUnfiltered = true;
    for (const auto& unique : uniques) {
        if (gvk::string::contains(logEntry, unique)) {
            uniqueOrUnfiltered &= encounteredUniques.insert(unique).second;
        }
    }
    return uniqueOrUnfiltered;
}

void output_filtered_log(std::ostream& ostrm, const std::vector<std::string>& logEntries, const std::set<std::string>& includes, const std::set<std::string>& excludes, const std::set<std::string>& uniques)
{
    std::set<std::string> encounteredUniques;
    for (auto logEntry : logEntries) {
        logEntry = gvk::string::remove_control_characters(logEntry, true);
        if (!logEntry.empty() && !log_entry_contains(logEntry, excludes) &&
            (includes.empty() || log_entry_contains(logEntry, includes)) &&
            (uniques.empty() || log_entry_unique_or_unfiltered(logEntry, uniques, encounteredUniques))) {
            ostrm << std::endl << logEntry << std::endl;
        }
    }
}

void output_function_counts(std::ostream& ostrm, const std::vector<std::string>& logEntries)
{
    std::map<std::string, uint32_t> functionCounts;
    for (const auto& logEntry : logEntries) {
        if (gvk::string::contains(logEntry, "Thread") && gvk::string::contains(logEntry, "Frame")) {
            auto functionNameBegin = logEntry.find_first_of(':') + 1;
            if (functionNameBegin != std::string::npos && functionNameBegin < logEntry.length() - 1) {
                functionNameBegin = logEntry.find_first_not_of(gvk::string::WhiteSpaceCharacters, functionNameBegin);
                auto functionNameEnd = logEntry.find_first_of('(');
                auto functionNameLength = functionNameEnd - functionNameBegin;
                auto functionName = logEntry.substr(functionNameBegin, functionNameLength);
                ++functionCounts[functionName];
            }
        }
    }
    for (const auto& functionCountItr : functionCounts) {
        ostrm << functionCountItr.first << " : " << functionCountItr.second << std::endl;
    }
}

void output_help_text()
{
    std::cout << "-f : Filepath to the log to parse; required" << std::endl;
    std::cout << "-i : Comma seperated list of strings to include" << std::endl;
    std::cout << "-x : Comma seperated list of strings to exclude" << std::endl;
    std::cout << "-u : Comma seperated list of strings to include once" << std::endl;
    std::cout << "-o : Output filepath" << std::endl;
    std::cout << std::endl;
    std::cout << "If any of -i, -x, or -u is specified, output will consist of all log entries matching the given includes/excludes" << std::endl;
    std::cout << "If none of -i, -x, nor -u is specified, output will consist of a list of all of the counts of each Vulkan function's use in the given log" << std::endl;
}

using CmdLine = std::map<std::string, std::string>;
CmdLine get_cmd_line(int argc, const char* ppArgv[], CmdLine cmdLine = { })
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

std::set<std::string> split(const std::string& values)
{
    std::set<std::string> uniqueValues;
    for (const auto& value : gvk::string::split(values, ",")) {
        if (2 < value.size() && value[0] == '0' && gvk::string::to_lower(value[1]) == 'x') {
            auto handle = value.substr(value.find_first_not_of("0Xx"));
            uniqueValues.insert(gvk::string::to_lower(handle));
            uniqueValues.insert(gvk::string::to_upper(handle));
        } else {
            uniqueValues.insert(value);
        }
    }
    return uniqueValues;
}

int main(int argc, const char* ppArgv[])
{
    auto cmdLine = get_cmd_line(argc, ppArgv);
    auto filepath = cmdLine["-f"];
    auto includes = cmdLine["-i"];
    auto excludes = cmdLine["-x"];
    auto uniques = cmdLine["-u"];
    auto output = cmdLine["-o"];

    if (!filepath.empty()) {
        // parse_log() outputs progress via std::cout, so format is set and reset before
        //  and after calling the function.
        std::ios_base::fmtflags stdCoutFmtFlags(std::cout.flags());
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        auto logEntries = parse_log(filepath);
        std::cout.flags(stdCoutFmtFlags);

        // Direct output to a specififed file or std::cout
        std::ofstream ofstrm(output);
        auto& ostrm = ofstrm.is_open() ? ofstrm : std::cout;

        // Output the cmd line used
        for (int i = 0; i < argc; ++i) {
            ostrm << ppArgv[i] << ' ';
        }
        ostrm << std::endl;

        // Output the selected info
        if (!includes.empty() || !excludes.empty() || !uniques.empty()) {
            output_filtered_log(ostrm, logEntries, split(includes), split(excludes), split(uniques));
        } else {
            output_function_counts(ostrm, logEntries);
        }
    } else {
        output_help_text();
    }
    return 0;
}
