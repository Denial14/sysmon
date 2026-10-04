#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace sysmon::parsers{
    struct CpuTimes{
        uint64_t user = 0;
        uint64_t nice = 0;
        uint64_t system = 0;
        uint64_t idle = 0;
        uint64_t iowait = 0;
        uint64_t irq = 0;
        uint64_t softirq = 0;
        uint64_t steal = 0;
        uint64_t guest = 0;
        uint64_t guest_nice = 0;

        uint64_t total_active() const{
            return user + nice + system + irq + softirq + steal;
        }

        uint64_t total() const{
            return total_active() + idle + iowait;
        }
    };

    struct CpuSnapshot{
        CpuTimes total;
        std::vector<CpuTimes> cores;
    };

    CpuSnapshot parse_cpu_stat();
}