#include "monitor.hpp"
#include <chrono>
#include <unordered_map>
#include <thread>
#include <algorithm>
#include <cmath>

namespace sysmon::core {

int64_t Monitor::now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()
    ).count();
}

Monitor::Monitor() = default;

Monitor::RawSnapshot Monitor::collect_raw() {
    RawSnapshot snap;
    snap.timestamp_ms = now_ms();
    snap.cpu = parsers::parse_cpu_stat();
    snap.mem = parsers::parse_meminfo();

    auto pids = parsers::list_pids();
    snap.processes.reserve(pids.size());
    for (int pid : pids) {
        parsers::ProcessInfo info;
        if (parsers::parse_process(pid, info)) {
            snap.processes.push_back(std::move(info));
        }
    }
    return snap;
}

Snapshot Monitor::take_snapshot() {
    RawSnapshot cur = collect_raw();

    Snapshot result;
    result.timestamp_ms = cur.timestamp_ms;

    result.mem_total_kb     = cur.mem.total_kb;
    result.mem_used_kb      = cur.mem.used_kb();
    result.mem_available_kb = cur.mem.available_kb;
    result.swap_total_kb    = cur.mem.swap_total_kb;
    result.swap_used_kb     = cur.mem.swap_used_kb();

    double mem_pct = 0.0, swap_pct = 0.0;
    if (result.mem_total_kb > 0) {
        mem_pct  = 100.0 * result.mem_used_kb / result.mem_total_kb;
    }
    if (result.swap_total_kb > 0) {
        swap_pct = 100.0 * result.swap_used_kb / result.swap_total_kb;
    }

    double cpu_pct = 0.0;
    if (has_prev_) {
        uint64_t total_delta  = cur.cpu.total.total()        - prev_.cpu.total.total();
        uint64_t active_delta = cur.cpu.total.total_active() - prev_.cpu.total.total_active();

        if (total_delta > 0) {
            cpu_pct = 100.0 * active_delta / total_delta;
        }
        result.cpu_total_percent = cpu_pct;

        size_t n = std::min(cur.cpu.cores.size(), prev_.cpu.cores.size());
        result.cpu_per_core_percent.resize(n);

        if (cores_history_.size() != n) {
            cores_history_.clear();
            cores_history_.resize(n);
        }

        for (size_t i = 0; i < n; ++i) {
            uint64_t td = cur.cpu.cores[i].total()        - prev_.cpu.cores[i].total();
            uint64_t ad = cur.cpu.cores[i].total_active() - prev_.cpu.cores[i].total_active();
            if (td > 0) {
                double core_pct = 100.0 * ad / td;
                result.cpu_per_core_percent[i] = core_pct;
                cores_history_[i].push(core_pct);
            } else {
                cores_history_[i].push(0.0);
            }
        }
    }

    cpu_history_.push(cpu_pct);
    mem_history_.push(mem_pct);
    swap_history_.push(swap_pct);

    std::unordered_map<int, uint64_t> prev_cpu_time;
    if (has_prev_) {
        prev_cpu_time.reserve(prev_.processes.size());
        for (const auto& p : prev_.processes) {
            prev_cpu_time[p.pid] = p.cpu_time();
        }
    }

    uint64_t system_total_delta = 0;
    if (has_prev_) {
        system_total_delta = cur.cpu.total.total() - prev_.cpu.total.total();
    }

    double num_cores = static_cast<double>(cur.cpu.cores.size());
    if (num_cores < 1.0) num_cores = 1.0;

    result.processes.reserve(cur.processes.size());
    for (const auto& p : cur.processes) {
        ProcessMetrics m;
        m.pid          = p.pid;
        m.name         = p.name;
        m.state        = p.state;
        m.ppid         = p.ppid;
        m.num_threads  = p.num_threads;
        m.rss_bytes    = p.rss_bytes;
        m.vsize_bytes  = p.vsize_bytes;
        m.cpu_time_ticks = p.cpu_time();

        if (has_prev_ && system_total_delta > 0) {
            auto it = prev_cpu_time.find(p.pid);
            if (it != prev_cpu_time.end()) {
                uint64_t proc_delta = p.cpu_time() - it->second;
                m.cpu_percent = 100.0 * proc_delta / system_total_delta * num_cores;
            }
        }

        if (result.mem_total_kb > 0) {
            m.memory_percent = 100.0 * (m.rss_bytes / 1024.0) / result.mem_total_kb;
        }

        result.processes.push_back(std::move(m));
    }

    prev_ = std::move(cur);
    has_prev_ = true;

    return result;
}

}