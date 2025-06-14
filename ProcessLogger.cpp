#include "header_files/ProcessLogger.h"
#include "header_files/Process.h"

void ProcessLogger::addLog(const Process& p, ProcessState state, int coreId) {
    log_entries.emplace_back(p, state, coreId);
}

void ProcessLogger::printAllLogs() const {
    std::cout << "\n--- Process Log ---" << std::endl;
    if (log_entries.empty()) {
        std::cout << "No log entries yet." << std::endl;
    } else {
        for (const auto& entry : log_entries) {
            entry.print();
        }
    }
    std::cout << "-------------------\n" << std::endl;
}

std::vector<ProcessLogEntry> ProcessLogger::getLogsByState(ProcessState state) const {
    std::vector<ProcessLogEntry> filtered_logs;
    for (const auto& entry : log_entries) {
        if (entry.state_at_log == state) {
            filtered_logs.push_back(entry);
        }
    }
    return filtered_logs;
}