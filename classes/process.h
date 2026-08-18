// classes/Process.h
#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <memory> 
#include <mutex>
#include <inttypes.h>
#include "ICommand.h"
#include "PageTable.h"

enum class ProcessState {
    IDLE,
    WAITING,
    RUNNING,
    FINISHED,
    TERMINATED 
};

std::string processStateToString(ProcessState state);
const int MAX = 1024;

class Process {
private:
    uint16_t pid;
    std::string process_name;
    std::vector<std::unique_ptr<ICommand>> instructions;
    // std::chrono::time_point<std::chrono::system_clock> start_time;       //we should be counting time according to hypothetical CPU ticks
    // std::chrono::time_point<std::chrono::system_clock> end_time;         //not actual system time
    uint64_t arrival_time;          //do we just compute for this during runtime and not store it in a variable?
    uint64_t burst_time;            //this one as well
    uint64_t remaining_burst;       //i feel like this is necessary kasi
    uint64_t waiting_time;          //no need i think
    uint64_t start_time[MAX];       //arbitrary size of 1024
    uint64_t end_time[MAX];         //arbitrary size of 1024
    size_t run_count;
    size_t program_counter;

    int current_core_id;            //need -1 for unassigned core
    ProcessState state;

    struct SymbolTableEntry {
        std::string name;
        uint16_t value;
        bool in_use = false;
    };
    SymbolTableEntry symbol_table[32]; 
    size_t symbol_count = 0;

    size_t memory_size; 
    std::unique_ptr<PageTable> page_table;
    std::string termination_reason = ""; 


    std::string creation_timestamp_str; 

    std::vector<uint16_t> memory_space;

    std::vector<std::string> logs;
    std::mutex logMutex;

public:
    Process();
    Process(int id, const std::string& name, size_t mem_size, size_t page_size);
    ~Process();

    void addInstruction(std::unique_ptr<ICommand> instruction);
    void runInstructionSlice(unsigned int slice_size);
    void runInstructions();

    void addLog(const std::string& message);
    std::vector<std::string> getLogs() const;

    const std::vector<std::unique_ptr<ICommand>>& getInstructions() const;
    int getInstructionCount() const;
    uint16_t getPid() const;
    std::string getProcessName() const;
    // std::chrono::time_point<std::chrono::system_clock> getStartTime() const;
    // std::chrono::time_point<std::chrono::system_clock> getEndTime() const;
    uint16_t getArrivalTime() const;
    uint16_t getBurstTime() const;
    uint16_t getRemainingBurst() const;
    uint16_t getWaitingTime() const;
    uint64_t getStartTime(int index) const;
    uint64_t getEndTime(int index) const;
    uint16_t getRunCount() const;
    int getCurrentCoreId() const;
    ProcessState getState() const;
    std::unordered_map<std::string, uint16_t> getVariables() const;
    uint16_t getVariableValue(const std::string& name) const;
    std::string getCreationTimestamp() const;
    size_t getProgramCounter() const;
    bool getVariable(const std::string& name, uint16_t& value) const;
    size_t getMemorySize() const;
    PageTable* getPageTable() const;

    void setBurstTime();
    void setBurstTime(uint64_t burst);
    void setRemainingBurst(uint64_t remaining);
    void setWaitingTime(uint64_t waiting);
    void setStartTime(uint64_t start);
    void setEndTime(uint64_t end);
    void setCurrentCoreId(int coreId);
    void setState(ProcessState newState);
    bool setVariable(const std::string& name, uint16_t value);
    void terminate(const std::string& reason); 
    std::string getTerminationReason() const;

    std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp);
    void displayInstructionList() const;
    void displayVariables() const;

    double updateRunningAverage(double previous_average, uint64_t new_wait, size_t index);

    uint16_t readMemory(uint32_t address) const;
    void writeMemory(uint32_t address, uint16_t value);

    void runScreenInterface(); 
};
