#include "../utils/file_reader.hpp"
#include "cpu.hpp"
#include <sstream>

namespace sysmon::parsers {
    static CpuTimes parse_line(std::istringstream& iss){
        CpuTimes t;
        iss >> t.user >> t.nice >> t.system >> t.idle >> t.iowait >> t.irq >> t.softirq >> t.steal >> t.guest >> t.guest_nice;
        return t;
    }

    CpuSnapshot parse_cpu_stat(){
        CpuSnapshot snap;
        auto lines_opt = utils::read_lines("/proc/stat");
        if (!lines_opt)
            return snap;
        
        for (const auto& line : *lines_opt){
            if (line.rfind("cpu", 0) != 0)
                continue;
            
            std::istringstream iss(line);
            std::string label;
            iss >> label;

            if (label == "cpu")
                snap.total = parse_line(iss);
            else
                snap.cores.push_back(parse_line(iss));
        }

        return snap;
    }
}