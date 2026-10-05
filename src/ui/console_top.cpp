#include "console_top.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <cmath>

namespace sysmon::ui {

static termios g_original_termios;
static bool g_termios_saved = false;

static void enable_raw_mode() {
    if (!g_termios_saved) {
        tcgetattr(STDIN_FILENO, &g_original_termios);
        g_termios_saved = true;
    }
    termios raw = g_original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

static void disable_raw_mode() {
    if (g_termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_original_termios);
    }
}

static void clear_screen() {
    std::cout << "\033[2J\033[H";
}

template<typename Buf>
static std::string sparkline(const Buf& buf, double max_value = 100.0) {
    static const char* blocks[] = {
        "\u2581", "\u2582", "\u2583", "\u2584",
        "\u2585", "\u2586", "\u2587", "\u2588"
    };

    std::string out;
    out.reserve(buf.size() * 3);

    for (std::size_t i = 0; i < buf.size(); ++i) {
        double v = buf.at(i);
        if (v < 0) v = 0;
        if (v > max_value) v = max_value;

        int idx = static_cast<int>((v / max_value) * 7.0 + 0.5);
        if (idx < 0) idx = 0;
        if (idx > 7) idx = 7;

        out += blocks[idx];
    }
    return out;
}

template<typename Buf>
static std::string sparkline_auto(const Buf& buf) {
    static const char* blocks[] = {
        "\u2581", "\u2582", "\u2583", "\u2584",
        "\u2585", "\u2586", "\u2587", "\u2588"
    };

    if (buf.size() == 0) return "";

    double max_val = buf.max();

    static const double nice[] = {1, 2, 5, 10, 20, 25, 50, 75, 100,
                                  200, 500, 1000, 2000, 5000, 10000};
    double nice_max = 1.0;
    for (double n : nice) {
        if (n >= max_val) { nice_max = n; break; }
        nice_max = n;
    }

    if (nice_max < 10.0) nice_max = 10.0;

    std::string out;
    out.reserve(buf.size() * 3);
    for (std::size_t i = 0; i < buf.size(); ++i) {
        double v = buf.at(i);
        if (v < 0) v = 0;
        if (v > nice_max) v = nice_max;
        int idx = static_cast<int>((v / nice_max) * 7.0 + 0.5);
        if (idx < 0) idx = 0;
        if (idx > 7) idx = 7;
        out += blocks[idx];
    }
    return out;
}

static void print_cores_history(const core::Monitor& monitor,
                                std::size_t max_rows = 8) {
    const auto& hist = monitor.cores_history();
    if (hist.empty()) return;

    std::size_t rows = std::min(hist.size(), max_rows);
    for (std::size_t i = 0; i < rows; ++i) {
        std::cout << "core " << std::setw(2) << i << ": "
                  << sparkline(hist[i], 100.0) << "\n";
    }
    if (hist.size() > max_rows) {
        std::cout << "... + " << (hist.size() - max_rows) << " more cores\n";
    }
    std::cout << "\n";
}

static void print_header(const core::Monitor& monitor, const core::Snapshot& snap) {
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "=== sysmon ===  "
              << "CPU: " << snap.cpu_total_percent << "%  |  "
              << "RAM: " << snap.mem_used_kb / 1024 << " / "
              << snap.mem_total_kb / 1024 << " MB ("
              << 100.0 * snap.mem_used_kb / snap.mem_total_kb << "%)  |  "
              << "Processes: " << snap.processes.size() << "\n\n";

    std::cout << "CPU: " << sparkline_auto(monitor.cpu_history()) << "\n";
    std::cout << "RAM: " << sparkline(monitor.mem_history(), 100.0) << "\n\n";

    print_cores_history(monitor);
}

enum class SortBy { Cpu, Mem, Pid };

static void print_processes(const core::Snapshot& snap, SortBy sort_by) {
    auto procs = snap.processes;
    std::sort(procs.begin(), procs.end(),
              [sort_by](const auto& a, const auto& b) {
                  switch (sort_by) {
                      case SortBy::Cpu: return a.cpu_percent > b.cpu_percent;
                      case SortBy::Mem: return a.rss_bytes > b.rss_bytes;
                      case SortBy::Pid: return a.pid < b.pid;
                  }
                  return false;
              });

    std::cout << std::left
              << std::setw(8)  << "PID"
              << std::setw(24) << "NAME"
              << std::setw(10) << "CPU%"
              << std::setw(10) << "MEM%"
              << std::setw(12) << "RSS(MB)"
              << std::setw(6)  << "STATE"
              << "\n";
    std::cout << std::string(70, '-') << "\n";

    size_t limit = std::min<size_t>(30, procs.size());
    for (size_t i = 0; i < limit; ++i) {
        const auto& p = procs[i];
        std::cout << std::setw(8)  << p.pid
                  << std::setw(24) << p.name.substr(0, 23)
                  << std::setw(10) << p.cpu_percent
                  << std::setw(10) << p.memory_percent
                  << std::setw(12) << p.rss_bytes / (1024 * 1024)
                  << std::setw(6)  << p.state
                  << "\n";
    }

    std::cout << "\n[q] Quit  [p] Sort by CPU  [m] Sort by Memory  [i] Sort by PID\n";
}

void run_console_top(core::Monitor& monitor, int interval_ms) {
    enable_raw_mode();
    std::cout << "\033[?25l";

    SortBy sort_by = SortBy::Cpu;
    bool running = true;

    monitor.take_snapshot();

    while (running) {
        auto snap = monitor.take_snapshot();

        clear_screen();
        print_header(monitor, snap);
        print_processes(snap, sort_by);
        std::cout.flush();

        int elapsed = 0;
        while (elapsed < interval_ms && running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            elapsed += 50;

            char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                switch (c) {
                    case 'q': running = false; break;
                    case 'p': sort_by = SortBy::Cpu; break;
                    case 'm': sort_by = SortBy::Mem; break;
                    case 'i': sort_by = SortBy::Pid; break;
                }
            }
        }
    }

    std::cout << "\033[?25h";
    disable_raw_mode();
    std::cout << "\n";
}

}