// classes/Process.h
#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include "ICommand.h" // Forward or full include, see below

enum class ProcessState {
    IDLE,
    WAITING,
    RUNNING,
    FINISHED
};

std::string processStateToString(ProcessState state);

class Process {
private:
    int pid;
    std::string process_name;
    std::vector<ICommand*> instructions;
    std::chrono::time_point<std::chrono::system_clock> start_time;
    std::chrono::time_point<std::chrono::system_clock> end_time;
    int current_core_id;
    ProcessState state;
    std::unordered_map<std::string, uint16_t> variables;

public:
    Process();
    Process(int id, const std::string& name);
    ~Process();

    void addInstruction(ICommand* instruction);
    void runInstructions();
    const std::vector<ICommand*>& getInstructions() const;
    int getPid() const;
    std::string getProcessName() const;
    std::chrono::time_point<std::chrono::system_clock> getStartTime() const;
    std::chrono::time_point<std::chrono::system_clock> getEndTime() const;
    int getCurrentCoreId() const;
    void setCurrentCoreId(int coreId);
    ProcessState getState() const;
    void setState(ProcessState newState);
    uint16_t getVariable(const std::string& name);
    void setVariable(const std::string& name, uint16_t value);
    std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp);
};
