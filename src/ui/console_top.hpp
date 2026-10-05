#pragma once
#include "../core/monitor.hpp"

namespace sysmon::ui {
void run_console_top(core::Monitor& monitor, int interval_ms = 1000);
}