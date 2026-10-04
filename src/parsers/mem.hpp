#pragma once
#include <cstdint>

namespace sysmon::parsers{
    struct MemInfo{
        uint64_t total_kb = 0;
        uint64_t free_kb = 0;
        uint64_t available_kb = 0;
        uint64_t buffers_kb = 0;
        uint64_t cached_kb = 0;
        uint64_t swap_total_kb = 0;
        uint64_t swap_free_kb = 0;

        uint64_t used_kb()const{
            return total_kb - available_kb;
        }

        uint64_t swap_used_kb()const{
            return swap_total_kb - swap_free_kb;
        }
    };

    MemInfo parse_meminfo();
}