#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include "classes/InstructionGenerator.cpp"

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
        int id;
        std::string name;
        std::vector<std::unique_ptr<ICommand>> instructions;
        ProcessState state;
        std::unordered_map<std::string, uint16_t> variables; // Each process has its own variables

    public:
        Process(int id, const std::string& name, int instruction_count) 
            : id(id), name(name), state(ProcessState::IDLE) {}
        
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
        
        const std::string& getName() const { return name; }
        int getId() const { return id; }
        
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
};
