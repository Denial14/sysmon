#include "mem.hpp"
#include "../utils/file_reader.hpp"
#include <sstream>
#include <string>
#include <unordered_map>

namespace sysmon::parsers{
    MemInfo parse_meminfo(){
        MemInfo info;

        auto lines_opt = utils::read_lines("/proc/meminfo");
        if (!lines_opt) return info;

        for (const auto& line : *lines_opt){
            std::stringstream iss(line);
            std::string uint;
            std::string key;
            uint64_t value;

            if (!(key.empty()) && key.back() == ':') 
                key.pop_back();

            if (key == "MemTotal")           info.total_kb = value;
            else if (key == "MemFree")       info.free_kb = value;
            else if (key == "MemAvailable")  info.available_kb = value;
            else if (key == "Buffers")       info.buffers_kb = value;
            else if (key == "Cached")        info.cached_kb = value;
            else if (key == "SwapTotal")     info.swap_total_kb = value;
            else if (key == "SwapFree")      info.swap_free_kb = value;
        }

        return info;
    }
}