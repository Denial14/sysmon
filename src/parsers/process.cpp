#include "process.hpp"
#include "../utils/file_reader.hpp"
#include <sstream>
#include <string>
#include <unistd.h>
#include <dirent.h>
#include <cstring>

namespace sysmon::parsers{

    std::vector<int> list_pids() {
        std::vector<int> pids;
        DIR* dir = opendir("/proc");
        if (!dir)
            return pids;

        dirent* entry;
        while ((entry = readdir(dir)) != nullptr){
            char* end = nullptr;
            long pid = strtol(entry->d_name, &end, 10);
            if (*end == '\0' && pid > 0) {
                pids.push_back(static_cast<int>(pid));
            }
        }
        closedir(dir);
        return pids;
    }

    bool parse_process(int pid, ProcessInfo& out){
        std::string path = "/proc/" + std::to_string(pid) + "/stat";
        auto content_opt = utils::read_file(path);
        if (!content_opt) return false;

        const std::string& s = *content_opt;

        size_t lparen = s.find('(');
        size_t rparen = s.rfind(')');
        if (lparen == std::string::npos || rparen == std::string::npos || rparen < lparen)
            return false;

        out.pid = std::stoi(s.substr(0, lparen));
        out.name = s.substr(lparen + 1, rparen - lparen - 1);

        std::string rest = s.substr(rparen + 2);
        std::istringstream iss(rest);
        iss >> out.state >> out.ppid;

        for (int i = 5; i <= 13; i++){
            std::string skip;
            iss >> skip;
        }

        iss >> out.utime >> out.stime;

        for (int i = 16; i <= 19; i++){
            std::string skip;
            iss >> skip;
        }

        iss >> out.num_threads;
        for (int i = 21; i <= 22; i++){
            std:: string skip;
            iss >> skip;
        }

        iss >> out.vsize_bytes >> out.rss_bytes;

        long page_size = sysconf(_SC_PAGESIZE);
        out.rss_bytes *= static_cast<uint64_t>(page_size);

        return true;
    }
}