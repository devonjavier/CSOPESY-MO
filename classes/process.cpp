// classes/Process.cpp
#include "process.h"
#include "ICommand.h"
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

Process::Process(int id, const std::string& name, size_t mem_size, size_t page_size) 
    : pid(id), 
      process_name(name), 
      current_core_id(-1), 
      state(ProcessState::IDLE),  
      program_counter(0),
      memory_size(mem_size)  {
            this->page_table = std::make_unique<PageTable>(mem_size, page_size);

            // Initialize other members as before

            if (mem_size > 0) {
                memory_space.resize(mem_size / 2, 0); 
            }

            arrival_time = 0;
            burst_time = 0;
            remaining_burst = 0;
            waiting_time = 0;
            run_count = 0;
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

std::vector<std::string> Process::getLogs() const {
    return logs;
}

uint16_t Process::getVariableValue(const std::string& name) const { // Made const correct
    // Linearly search the symbol table for the variable name.
    for (size_t i = 0; i < symbol_count; ++i) {
        if (symbol_table[i].in_use && symbol_table[i].name == name) {
            return symbol_table[i].value;
        }
    }

    return 0;
}

bool Process::getVariable(const std::string& name, uint16_t& value) const {
    for (size_t i = 0; i < symbol_count; ++i) {
        if (symbol_table[i].in_use && symbol_table[i].name == name) {
            value = symbol_table[i].value;
            return true; // Found the variable
        }
    }
    return false; // Variable not found
}

size_t Process::getProgramCounter() const {
    return this->program_counter;
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

bool Process::setVariable(const std::string& name, uint16_t value) {
    for (size_t i = 0; i < symbol_count; ++i) {
        if (symbol_table[i].in_use && symbol_table[i].name == name) {
            symbol_table[i].value = value;
            return true; 
        }
    }

    if (symbol_count >= 32) {
        addLog("[System] Symbol table full. Declaration of '" + name + "' ignored.");
        return false;
    }


    symbol_table[symbol_count].name = name;
    symbol_table[symbol_count].value = value;
    symbol_table[symbol_count].in_use = true;
    symbol_count++; 
    
    return true; 
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
    std::cout << "--- Symbol Table for PID " << this->pid << " ---\n";
    if (symbol_count == 0) {
        std::cout << "  (empty)\n";
        return;
    }

    // Iterate only through the variables that are currently in use.
    for (size_t i = 0; i < symbol_count; ++i) {
        if (symbol_table[i].in_use) {
            std::cout << "  [" << i << "] " << symbol_table[i].name 
                      << " = " << symbol_table[i].value << std::endl;
        }
    }
    std::cout << "------------------------------------\n";
}

void Process::displayInstructionList() const { // Made const correct
    for (size_t i = 0; i < instructions.size(); ++i) {
        std::cout << "[ICommand #" << i << "] "
                  << instructions[i]->toString(*this) 
                  << "\n";
    }
}

double updateRunningAverage(double previous_average, uint64_t new_wait, size_t index) {
    return (previous_average * (double)index + new_wait) / (index + 1);
}

void Process::addLog(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    logs.push_back(message);
}

void Process::runScreenInterface() {
    std::string command;
    while (true) {
        clear_screen(); // Assumes this is a global function

        // --- Print all the information in the correct order ---
        std::cout << "Process name: " << this->getProcessName() << std::endl;
        std::cout << "ID: " << this->getPid() << std::endl;
        
        std::cout << "Logs:" << std::endl;
        {
            std::lock_guard<std::mutex> lock(this->logMutex);
            if (logs.empty()) {
                std::cout << "  (No log entries yet.)\n";
            } else {
                for (const auto& log_entry : logs) {
                    std::cout << log_entry << std::endl;
                }
            }
        }
        
        std::cout << std::endl; // Add a blank line for spacing

        // Check the state to decide what to print at the end
        if (this->getState() == ProcessState::FINISHED) {
            std::cout << "Finished!" << std::endl;
        } else {
            std::cout << "Current instruction line: " << this->program_counter << std::endl;
            std::cout << "Lines of code: " << this->getInstructionCount() << std::endl;
        }

        std::cout << "\nroot:\\> "; // Mimic the prompt from the image
        
        std::getline(std::cin, command);

        if (command == "exit") {
            break; // Exit this screen and return to the main menu
        } else if (command == "process-smi") {
            // The loop will automatically refresh, so we just continue
            continue;
        } else {
            std::cout << "Unknown command. Use 'process-smi' to refresh or 'exit' to return." << std::endl;
            system("pause");
        }
    }
}

uint16_t Process::readMemory(uint32_t address) const {
    // The address from the command is a byte address. We need to convert
    // it to an index into our vector of 2-byte words.
    size_t index = address / 2;

    // Boundary check
    if (index >= memory_space.size()) {
        // This case should be caught by the ICommand's access violation check,
        // but it's good practice to have a safe fallback.
        return 0; 
    }
    return memory_space[index];
}

void Process::writeMemory(uint32_t address, uint16_t value) {
    size_t index = address / 2;

    // Boundary check
    if (index >= memory_space.size()) {
        // Again, this is a safety fallback. The ICommand should prevent this.
        return;
    }
    memory_space[index] = value;
}

size_t Process::getMemorySize() const {
    return memory_size;
}

PageTable* Process::getPageTable() const {
    return page_table.get(); 
}

void Process::terminate(const std::string& reason) {
    this->state = ProcessState::TERMINATED;
    this->termination_reason = reason;
    // You might also want to log this event
    addLog("[System] Process terminated. Reason: " + reason);
}

std::string Process::getTerminationReason() const {
    return termination_reason;
}

