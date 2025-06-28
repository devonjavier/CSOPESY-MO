#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <limits>
#include <algorithm>
#include "InstructionGenerator.cpp"

enum class ProcessState{
    IDLE,
    WAITING,
    RUNNING,
    FINISHED
};

std::string processStateToString(ProcessState state) { 
    switch(state) {
        case ProcessState::IDLE:
            return "IDLE";
        case ProcessState::WAITING:
            return "WAITING";
        case ProcessState::RUNNING:
            return "RUNNING";
        case ProcessState::FINISHED:
            return "FINISHED";
        default: return "UNKNOWN";
    }
}

class Process {
private:
    int pid;                    // Process ID
    std::string processName;    // Process name
    std::vector<std::unique_ptr<ICommand>> instructions;
    ProcessState state;
    std::unordered_map<std::string, uint16_t> variables; // Each process has its own variables
    
    // Execution tracking
    size_t currentInstructionIndex;
    int currentCoreId;
    
    // Timing information
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::time_point<std::chrono::system_clock> endTime;
    
    // Instruction generation
    InstructionGenerator instructionGen;

public:
    Process(int id, const std::string& name, int instruction_count) 
        : pid(id), processName(name), state(ProcessState::IDLE), 
          currentInstructionIndex(0), currentCoreId(-1),
          startTime{}, endTime{} {
        
        // Generate random instructions
        for (int i = 0; i < instruction_count; ++i) {
            instructions.push_back(instructionGen.generateRandomInstruction());
        }
    }
    
    // Copy constructor for proper copying of unique_ptr instructions
    Process(const Process& other) 
        : pid(other.pid), processName(other.processName), state(other.state),
          variables(other.variables), currentInstructionIndex(other.currentInstructionIndex),
          currentCoreId(other.currentCoreId), startTime(other.startTime), endTime(other.endTime) {
        
        // Deep copy instructions (this is a simplified approach - you might need to implement proper cloning)
        // For now, regenerate instructions with same count
        InstructionGenerator gen;
        for (size_t i = 0; i < other.instructions.size(); ++i) {
            instructions.push_back(gen.generateRandomInstruction());
        }
    }
    
    // Assignment operator
    Process& operator=(const Process& other) {
        if (this != &other) {
            pid = other.pid;
            processName = other.processName;
            state = other.state;
            variables = other.variables;
            currentInstructionIndex = other.currentInstructionIndex;
            currentCoreId = other.currentCoreId;
            startTime = other.startTime;
            endTime = other.endTime;
            
            // Clear and regenerate instructions
            instructions.clear();
            InstructionGenerator gen;
            for (size_t i = 0; i < other.instructions.size(); ++i) {
                instructions.push_back(gen.generateRandomInstruction());
            }
        }
        return *this;
    }
    
    // Getters required by the main OS interface
    int getPid() const { return pid; }
    const std::string& getProcessName() const { return processName; }
    ProcessState getState() const { return state; }
    int getCurrentCoreId() const { return currentCoreId; }
    
    std::chrono::time_point<std::chrono::system_clock> getStartTime() const { 
        return startTime; 
    }
    
    std::chrono::time_point<std::chrono::system_clock> getEndTime() const { 
        return endTime; 
    }
    
    // Setters required by the scheduler
    void setState(ProcessState newState) { state = newState; }
    void setCurrentCoreId(int coreId) { currentCoreId = coreId; }
    
    void setStartTime(const std::chrono::time_point<std::chrono::system_clock>& time) { 
        startTime = time; 
    }
    
    void setEndTime(const std::chrono::time_point<std::chrono::system_clock>& time) { 
        endTime = time; 
    }
    
    // Execution methods
    bool executeNextInstruction() {
        if (currentInstructionIndex >= instructions.size()) {
            return true; // Process completed
        }
        
        // Execute current instruction
        try {
            instructions[currentInstructionIndex]->execute(variables);
            currentInstructionIndex++;
            
            // Check if all instructions are completed
            return currentInstructionIndex >= instructions.size();
        } catch (const std::exception& e) {
            std::cerr << "Error executing instruction in process " << processName 
                      << ": " << e.what() << std::endl;
            currentInstructionIndex++; // Skip problematic instruction
            return currentInstructionIndex >= instructions.size();
        }
    }
    
    bool isCompleted() const {
        return currentInstructionIndex >= instructions.size();
    }
    
    size_t getCurrentInstructionIndex() const {
        return currentInstructionIndex;
    }
    
    size_t getTotalInstructions() const {
        return instructions.size();
    }
    
    double getProgress() const {
        if (instructions.empty()) return 100.0;
        return (static_cast<double>(currentInstructionIndex) / instructions.size()) * 100.0;
    }
    
    // Legacy methods for compatibility
    void addInstruction(std::unique_ptr<ICommand> instruction) {
        instructions.push_back(std::move(instruction));
    }
    
    const std::vector<std::unique_ptr<ICommand>>& getInstructions() const {
        return instructions;
    }
    
    std::unordered_map<std::string, uint16_t>& getVariables() {
        return variables;
    }
    
    size_t getInstructionCount() const {
        return instructions.size();
    }
    
    const std::string& getName() const { return processName; }
    int getId() const { return pid; }
    
    // Helper function to get/create variable (auto-declares with 0 if not exists)
    uint16_t getVariable(const std::string& var_name) {
        if (variables.find(var_name) == variables.end()) {
            variables[var_name] = 0;
        }
        return variables[var_name];
    }
    
    void setVariable(const std::string& var_name, uint16_t value) {
        // Clamp between 0 and max uint16
        variables[var_name] = std::clamp(value, static_cast<uint16_t>(0), 
                                         std::numeric_limits<uint16_t>::max());
    }
    

    void printStatus() const {
        std::cout << "Process " << processName << " (PID: " << pid << ")\n"
                  << "  State: " << processStateToString(state) << "\n"
                  << "  Core: " << (currentCoreId == -1 ? "Not assigned" : std::to_string(currentCoreId)) << "\n"
                  << "  Progress: " << currentInstructionIndex << "/" << instructions.size() 
                  << " (" << getProgress() << "%)\n";
    }
    
    // Reset process for re-execution (if needed)
    void reset() {
        currentInstructionIndex = 0;
        state = ProcessState::IDLE;
        currentCoreId = -1;
        variables.clear();
        startTime = {};
        endTime = {};
    }
};