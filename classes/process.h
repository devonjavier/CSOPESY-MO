// classes/Process.h
#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <inttypes.h>
#include "ICommand.h"

enum class ProcessState {
    IDLE,
    WAITING,
    RUNNING,
    FINISHED
};

std::string processStateToString(ProcessState state);

class Process {
private:
    uint16_t pid;
    std::string process_name;
    std::vector<ICommand*> instructions;
    // std::chrono::time_point<std::chrono::system_clock> start_time;       //we should be counting time according to hypothetical CPU ticks
    // std::chrono::time_point<std::chrono::system_clock> end_time;         //not actual system time
    uint64_t arrival_time;          //do we just compute for this during runtime and not store it in a variable?
    uint64_t burst_time;            //this one as well
    uint64_t remaining_burst;       //i feel like this is necessary kasi
    uint64_t waiting_time;          //no need i think
    uint64_t start_time[MAX];      //arbitrary size of 1024
    uint64_t end_time[MAX];        //arbitrary size of 1024
    size_t run_count;

    int current_core_id;            //need -1 for unassigned core
    ProcessState state;
    std::unordered_map<std::string, uint16_t> variables;

public:
    Process();
    Process(int id, const std::string& name);
    ~Process();

    void addInstruction(ICommand* instruction);
    void runInstructions();

    const std::vector<ICommand*>& getInstructions() const;
    uint16_t getPid() const;
    std::string getProcessName() const;
    // std::chrono::time_point<std::chrono::system_clock> getStartTime() const;
    // std::chrono::time_point<std::chrono::system_clock> getEndTime() const;
    uint16_t Process::getArrivalTime() const;
    uint16_t Process::getBurstTime() const;
    uint16_t Process::getRemainingBurst() const;
    uint16_t Process::getWaitingTime() const;
    uint64_t Process::getStartTime(int index) const;
    uint64_t Process::getEndTime(int index) const;
    uint16_t Process::getRunCount() const;
    int getCurrentCoreId() const;
    ProcessState getState() const;
    std::unordered_map<std::string, uint16_t> getVariables() const;
    uint16_t getVariableValue(const std::string& name);

    void setBurstTime();
    void setBurstTime(uint64_t burst);
    void setRemainingBurst(uint64_t remaining);
    void setWaitingTime(uint64_t waiting);
    void setStartTime(uint64_t start);
    void setEndTime(uint64_t end);
    void setCurrentCoreId(int coreId);
    void setState(ProcessState newState);
    void setVariable(const std::string& name, uint16_t value);

    std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp);
    void displayInstructionList();
    void displayVariables() const;

    double updateRunningAverage(double previous_average, uint64_t new_wait, size_t index);
};