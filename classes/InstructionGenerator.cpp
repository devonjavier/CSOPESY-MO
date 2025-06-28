#include <random>
#include <memory>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include "ICommand.cpp"

class InstructionGenerator {
private:
    std::default_random_engine generator;
    std::uniform_int_distribution<int> instruction_type_dist;
    std::uniform_int_distribution<uint16_t> value_dist;
    std::uniform_int_distribution<uint8_t> sleep_dist;
    std::uniform_int_distribution<uint8_t> loop_dist;
    int current_nest_level;
    
public:
    InstructionGenerator() : 
        generator(std::chrono::system_clock::now().time_since_epoch().count()),
        instruction_type_dist(0, 5),  // 6 instruction types (0-5)
        value_dist(1, 1000),          // Random values 1-1000
        sleep_dist(1, 10),            // Sleep 1-10 CPU ticks
        loop_dist(2, 5),              // Loop 2-5 times
        current_nest_level(0)
    {}
    
    std::unique_ptr<ICommand> generateRandomInstruction(int nest_level = 0) {
        current_nest_level = nest_level;
        int type = instruction_type_dist(generator);
        
        // Limit FOR loops if we're too deeply nested
        if (nest_level >= 3 && type == 5) {
            type = instruction_type_dist(generator) % 5; // Exclude FOR
        }
        
        switch(type) {
            case 0: return generatePRINT();
            case 1: return generateDECLARE();
            case 2: return generateADD();
            case 3: return generateSUBTRACT();
            case 4: return generateSLEEP();
            case 5: return generateFOR(nest_level);
            default: return std::make_unique<PRINT>();
        }
    }
    
private:
    std::unique_ptr<PRINT> generatePRINT() {
        std::uniform_int_distribution<int> choice(0, 1);
        if (choice(generator) == 0) {
            return std::make_unique<PRINT>(); // Default "Hello world from <process>"
        } else {
            return std::make_unique<PRINT>(generateRandomVarName()); // Print variable
        }
    }
    
    std::unique_ptr<DECLARE> generateDECLARE() {
        return std::make_unique<DECLARE>(generateRandomVarName(), value_dist(generator));
    }
    
    std::unique_ptr<ADD> generateADD() {
        std::uniform_int_distribution<int> choice(0, 3);
        std::string var1 = generateRandomVarName();
        
        switch(choice(generator)) {
            case 0: return std::make_unique<ADD>(var1, generateRandomVarName(), generateRandomVarName());
            case 1: return std::make_unique<ADD>(var1, generateRandomVarName(), value_dist(generator));
            case 2: return std::make_unique<ADD>(var1, value_dist(generator), generateRandomVarName());
            case 3: return std::make_unique<ADD>(var1, value_dist(generator), value_dist(generator));
            default: return std::make_unique<ADD>(var1, generateRandomVarName(), generateRandomVarName());
        }
    }
    
    std::unique_ptr<SUBTRACT> generateSUBTRACT() {
        std::uniform_int_distribution<int> choice(0, 3);
        std::string var1 = generateRandomVarName();
        
        switch(choice(generator)) {
            case 0: return std::make_unique<SUBTRACT>(var1, generateRandomVarName(), generateRandomVarName());
            case 1: return std::make_unique<SUBTRACT>(var1, generateRandomVarName(), value_dist(generator));
            case 2: return std::make_unique<SUBTRACT>(var1, value_dist(generator), generateRandomVarName());
            case 3: return std::make_unique<SUBTRACT>(var1, value_dist(generator), value_dist(generator));
            default: return std::make_unique<SUBTRACT>(var1, generateRandomVarName(), generateRandomVarName());
        }
    }
    
    std::unique_ptr<SLEEP> generateSLEEP() {
        return std::make_unique<SLEEP>(sleep_dist(generator));
    }
    
    std::unique_ptr<FOR> generateFOR(int nest_level) {
        std::vector<std::unique_ptr<ICommand>> loop_instructions;
        std::uniform_int_distribution<int> instr_count(1, 3); // 1-3 instructions in FOR loop
        int num_instructions = instr_count(generator);
        
        for (int i = 0; i < num_instructions; ++i) {
            loop_instructions.push_back(generateRandomInstruction(nest_level + 1));
        }
        
        return std::make_unique<FOR>(std::move(loop_instructions), loop_dist(generator));
    }
    
    std::string generateRandomVarName() {
        static const std::vector<std::string> var_names = {
            "x", "y", "z", "counter", "temp", "result", "value", "sum", "a", "b"
        };
        std::uniform_int_distribution<int> name_dist(0, var_names.size() - 1);
        return var_names[name_dist(generator)];
    }
};