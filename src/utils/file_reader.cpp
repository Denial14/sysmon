#include "file_reader.hpp"
#include <fstream>
#include <sstream>

namespace sysmon::utils{
    std::optional<std::string> read_file(const std::string &path){
        std::ifstream file(path);
        if (!file.is_open())
            return std::nullopt;
        
        std:: stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::optional<std::vector<std::string>> read_lines(const std::string &path){
        std::ifstream file(path);
        if (!file.is_open()) 
            return std::nullopt;

        std::vector<std::string> lines;
        std::string line;
        while(getline(file, line)){
            lines.push_back(std::move(line));
        }
        return lines;
    }
}