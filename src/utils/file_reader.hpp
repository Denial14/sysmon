#pragma once
#include<string>
#include<optional>
#include<vector>

namespace sysmon::utils{
    std::optional<std::string> read_file(const std::string &path);
    std::optional<std::vector<std::string>> read_lines(const std::string &path);
}