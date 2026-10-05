#pragma once
#include "snapshot.hpp"
#include "ring_buffer.hpp"
#include "../parsers/cpu.hpp"
#include "../parsers/mem.hpp"
#include "../parsers/process.hpp"

namespace sysmon::core {

constexpr std::size_t HISTORY_SIZE = 60;

class Monitor {
public:
    Monitor();

    Snapshot take_snapshot();

    const RingBuffer<double, HISTORY_SIZE>& cpu_history() const { return cpu_history_; }
    const RingBuffer<double, HISTORY_SIZE>& mem_history() const { return mem_history_; }
    const RingBuffer<double, HISTORY_SIZE>& swap_history() const { return swap_history_; }

    const std::vector<RingBuffer<double, HISTORY_SIZE>>& cores_history() const {
        return cores_history_;
    }

private:
    struct RawSnapshot {
        parsers::CpuSnapshot cpu;
        parsers::MemInfo mem;
        std::vector<parsers::ProcessInfo> processes;
        int64_t timestamp_ms = 0;
    };

    RawSnapshot prev_;
    bool has_prev_ = false;

    RingBuffer<double, HISTORY_SIZE> cpu_history_;
    RingBuffer<double, HISTORY_SIZE> mem_history_;
    RingBuffer<double, HISTORY_SIZE> swap_history_;
    std::vector<RingBuffer<double, HISTORY_SIZE>> cores_history_;

    RawSnapshot collect_raw();
    static int64_t now_ms();
};

}