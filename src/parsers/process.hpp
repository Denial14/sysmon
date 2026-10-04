#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace sysmon::parsers {

struct ProcessInfo {
    int pid = 0;
    std::string name;
    char state = '?';
    int ppid = 0;
    uint64_t utime = 0;
    uint64_t stime = 0;
    int num_threads = 0;
    uint64_t vsize_bytes = 0;
    uint64_t rss_bytes = 0;

    uint64_t cpu_time() const { return utime + stime; }
};

bool parse_process(int pid, ProcessInfo& out);

std::vector<int> list_pids();
}