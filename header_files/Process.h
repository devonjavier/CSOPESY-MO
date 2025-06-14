#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <chrono>
#include "ProcessState.h"

class Process {
private:
    int pid;
    std::string process_name;
    int priority;
    int burst_time;
    int remaining_burst_time;
    int waiting_time;
    int turnaround_time;
    std::chrono::time_point<std::chrono::system_clock> start_time;
    int current_core_id;
    ProcessState state;

public:
    Process(int id, const std::string& name, int prio, int burst);

    int getPid() const;
    std::string getProcessName() const;
    int getPriority() const;
    int getBurstTime() const;
    int getRemainingBurstTime() const;
    int getWaitingTime() const;
    int getTurnaroundTime() const;
    std::chrono::time_point<std::chrono::system_clock> getStartTime() const;
    int getCurrentCoreId() const;
    ProcessState getState() const;

    void setState(ProcessState newState, int coreId = -1);
    void displayProcess() const;
};

#endif // PROCESS_H