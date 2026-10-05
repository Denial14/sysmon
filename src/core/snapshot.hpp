#pragma once
#include "../parsers/process.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace sysmon::core {

struct ProcessMetrics {
    int pid = 0;
    std::string name;
    char state = '?';
    int ppid = 0;
    int num_threads = 0;

    uint64_t rss_bytes = 0;
    uint64_t vsize_bytes = 0;
    uint64_t cpu_time_ticks = 0;

    double cpu_percent = 0.0;
    double memory_percent = 0.0;
};

struct Snapshot {
    double cpu_total_percent = 0.0;
    std::vector<double> cpu_per_core_percent;

    uint64_t mem_total_kb = 0;
    uint64_t mem_used_kb = 0;
    uint64_t mem_available_kb = 0;
    uint64_t swap_total_kb = 0;
    uint64_t swap_used_kb = 0;

    std::vector<ProcessMetrics> processes;

    int64_t timestamp_ms = 0;
};

}