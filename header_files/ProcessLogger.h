#ifndef PROCESS_LOGGER_H
#define PROCESS_LOGGER_H

#include <vector>
#include <iostream>
#include "ProcessLogEntry.h" // Requires ProcessLogEntry definition
#include "ProcessState.h"    // Requires ProcessState enum

class ProcessLogger {
public:
    std::vector<ProcessLogEntry> log_entries;

    void addLog(const Process& p, ProcessState state, int coreId = -1);
    void printAllLogs() const;
    std::vector<ProcessLogEntry> getLogsByState(ProcessState state) const;
};

#endif