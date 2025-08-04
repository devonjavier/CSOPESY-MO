// classes/Process.cpp
#include "process.h"
#include "ICommand.cpp"
#include <iostream>

std::string processStateToString(ProcessState state) {
    switch (state) {
        case ProcessState::IDLE: return "IDLE";
        case ProcessState::WAITING: return "WAITING";
        case ProcessState::RUNNING: return "RUNNING";
        case ProcessState::FINISHED: return "FINISHED";
        default: return "UNKNOWN";
    }
}

Process::Process() : pid(-1), process_name("null"), current_core_id(-1), state(ProcessState::IDLE), next(nullptr), program_counter(0) {}

Process::Process(int id, const std::string& name)
    : pid(id), process_name(name), current_core_id(-1), state(ProcessState::IDLE), next(nullptr), program_counter(0) {
    arrival_time = 0;               //init to 0 or current time if needed
    burst_time = 0;                 //init to 0 or a default value
    remaining_burst = burst_time;   //init to burst_time
    waiting_time = 0;               //init to 0 or a default value
    run_count = 0;                  //init to 0
    
}

Process::~Process() {}

uint16_t Process::getPid() const {
    return pid;
}

std::string Process::getProcessName() const {
    return process_name;
}

const std::vector<std::unique_ptr<ICommand>>& Process::getInstructions() const {
    return instructions;
}

int Process::getInstructionCount() const {
    return instructions.size();
}

// uint64_t Process::getStartTime() const {
//     return start_time;
// }

// uint64_t Process::getEndTime() const {
//     return end_time;
// }

uint16_t Process::getArrivalTime() const {
    return arrival_time;
}

uint16_t Process::getBurstTime() const {
    return burst_time;
}

uint16_t Process::getRemainingBurst() const {
    return remaining_burst;
}

uint16_t Process::getWaitingTime() const {
    return waiting_time;
}

uint64_t Process::getStartTime(int index) const {
    if (index >= 0 && index < MAX) {
        return start_time[index];
    }
    return 0;
}

uint64_t Process::getEndTime(int index) const {
    if (index >= 0 && index < MAX) {
        return end_time[index];
    }
    return 0;
}

uint16_t Process::getRunCount() const {
    return run_count;
}

int Process::getCurrentCoreId() const {
    return current_core_id;
}

ProcessState Process::getState() const {
    return state;
}

std::unordered_map<std::string, uint16_t> Process::getVariables() const {
    return variables;
}

uint16_t Process::getVariableValue(const std::string& name) {
    return variables.find(name) != variables.end() ? variables[name] : 0;
}

void Process::setBurstTime() {
    burst_time = instructions.size();
}

void Process::setBurstTime(uint64_t burst) {
    burst_time = burst;
}

void Process::setStartTime(uint64_t start) {
    if (run_count >= MAX) {
        fprintf(stderr, "Error: Index out of bounds with run_count=%zu\n", run_count);
        return;
    }
    start_time[run_count] = start;
}

void Process::setEndTime(uint64_t end) {
    if (run_count >= MAX) {
        fprintf(stderr, "Error: Index out of bounds with run_count=%zu\n", run_count);
        return;
    }
    end_time[run_count] = end;
    run_count++;
}

void Process::setRemainingBurst(uint64_t remaining) {
    remaining_burst = remaining;
}
void Process::setWaitingTime(uint64_t waiting) {
    waiting_time = waiting;
}

void Process::setCurrentCoreId(int coreId) {
    current_core_id = coreId;
}

void Process::setState(ProcessState newState) {
    this->state = newState;
    // if (newState == ProcessState::RUNNING &&
    //     this->start_time.time_since_epoch().count() == 0) {
    //     this->start_time = std::chrono::system_clock::now();
    // }
}

void Process::setVariable(const std::string& name, uint16_t value) {
    variables[name] = value;
}

std::string Process::formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp) {
    if (tp.time_since_epoch().count() == 0) return "N/A";
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %I:%M:%S%p", std::localtime(&tt));
    return std::string(buffer);
}

void Process::addInstruction(std::unique_ptr<ICommand> instruction) {
    instructions.push_back(std::move(instruction));
}

void Process::runInstructionSlice(unsigned int slice_size) {
    if (state != ProcessState::RUNNING) return;

    for (unsigned int i = 0; i < slice_size && program_counter < instructions.size(); ++i) {
        instructions[program_counter]->execute(*this);
        program_counter++; 
    }
}

void Process::runInstructions() {
    for (const auto& cmdPtr : instructions) {
        if (cmdPtr) {
            cmdPtr->execute(*this);
        }
    }
    // end_time = std::chrono::system_clock::now();
}

void Process::displayVariables() const {
    for (const auto& pair : variables) {
        std::cout << "Key: "   << pair.first
                  << ", Value: " << pair.second
                  << std::endl;
    }
}

void Process::displayInstructionList() {
    for (size_t i = 0; i < instructions.size(); ++i) {

        std::cout
            << "[ICommand #" << i << "] "
            << instructions[i]->toString()
            << "\n";
    }
}

double updateRunningAverage(double previous_average, uint64_t new_wait, size_t index) {
    return (previous_average * (double)index + new_wait) / (index + 1);
}