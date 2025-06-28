// #pragma once
// #include <string>
// #include <unordered_map>
// #include <vector>
// #include <memory>
// #include <iostream>
// #include <thread>
// #include <chrono>

// // Forward declaration for Process if needed
// // class Process;

// // class Process; 
// #include "process.h"

// class ICommand {
//     public:
//         virtual ~ICommand() = default;

//         virtual void execute(Process& process) = 0;
// };

// class PRINT : public ICommand {
// private:
//     std::string message;
//     std::string variableName;
//     bool isVariable;

// public:
//     // Default constructor - prints "Hello world from <process>"
//     PRINT() : message(""), variableName(""), isVariable(false) {}
    
//     // Constructor for printing a variable
//     PRINT(const std::string& varName) : message(""), variableName(varName), isVariable(true) {}
    
//     // Constructor for printing a custom message
//     PRINT(const std::string& msg, bool isMsg) : message(msg), variableName(""), isVariable(false) {}

//     void execute(Process& process) override {
//         if (isVariable && !variableName.empty()) {
//             // Print variable value
//             uint16_t value = process.getVariable(variableName);
//             std::cout << "[Process " << process.getProcessName() << "] " 
//                       << variableName << " = " << value << std::endl;
//         } else if (!message.empty()) {
//             // Print custom message
//             std::cout << "[Process " << process.getProcessName() << "] " 
//                       << message << std::endl;
//         } else {
//             // Default message
//             std::cout << "[Process " << process.getProcessName() << "] " 
//                       << "Hello world from " << process.getProcessName() << std::endl;
//         }
//     }
// };

// class DECLARE : public ICommand {
// private:
//     std::string variableName;
//     uint16_t value;

// public:
//     DECLARE(const std::string& varName, uint16_t val) 
//         : variableName(varName), value(val) {}

//     void execute(Process& process) override {
//         process.setVariable(variableName, value);
//         std::cout << "[Process " << process.getProcessName() << "] " 
//                   << "Declared " << variableName << " = " << value << std::endl;
//     }
// };

// class ADD : public ICommand {
// private:
//     std::string resultVar;
//     std::string operand1Var;
//     std::string operand2Var;
//     uint16_t operand1Value;
//     uint16_t operand2Value;
//     bool operand1IsVar;
//     bool operand2IsVar;

// public:
//     // var1 = var2 + var3
//     ADD(const std::string& var1, const std::string& var2, const std::string& var3)
//         : resultVar(var1), operand1Var(var2), operand2Var(var3), 
//           operand1Value(0), operand2Value(0), operand1IsVar(true), operand2IsVar(true) {}
    
//     // var1 = var2 + value
//     ADD(const std::string& var1, const std::string& var2, uint16_t value)
//         : resultVar(var1), operand1Var(var2), operand2Var(""), 
//           operand1Value(0), operand2Value(value), operand1IsVar(true), operand2IsVar(false) {}
    
//     // var1 = value + var3
//     ADD(const std::string& var1, uint16_t value, const std::string& var3)
//         : resultVar(var1), operand1Var(""), operand2Var(var3), 
//           operand1Value(value), operand2Value(0), operand1IsVar(false), operand2IsVar(true) {}
    
//     // var1 = value1 + value2
//     ADD(const std::string& var1, uint16_t value1, uint16_t value2)
//         : resultVar(var1), operand1Var(""), operand2Var(""), 
//           operand1Value(value1), operand2Value(value2), operand1IsVar(false), operand2IsVar(false) {}

//     void execute(Process& process) override {
//         uint16_t op1 = operand1IsVar ? process.getVariable(operand1Var) : operand1Value;
//         uint16_t op2 = operand2IsVar ? process.getVariable(operand2Var) : operand2Value;
        
//         // Perform addition with overflow protection
//         uint32_t result = static_cast<uint32_t>(op1) + static_cast<uint32_t>(op2);
//         uint16_t finalResult = std::min(result, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()));
        
//         process.setVariable(resultVar, finalResult);
        
//         std::cout << "[Process " << process.getProcessName() << "] " 
//                   << resultVar << " = " << op1 << " + " << op2 << " = " << finalResult << std::endl;
//     }
// };

// class SUBTRACT : public ICommand {
// private:
//     std::string resultVar;
//     std::string operand1Var;
//     std::string operand2Var;
//     uint16_t operand1Value;
//     uint16_t operand2Value;
//     bool operand1IsVar;
//     bool operand2IsVar;

// public:
//     // var1 = var2 - var3
//     SUBTRACT(const std::string& var1, const std::string& var2, const std::string& var3)
//         : resultVar(var1), operand1Var(var2), operand2Var(var3), 
//           operand1Value(0), operand2Value(0), operand1IsVar(true), operand2IsVar(true) {}
    
//     // var1 = var2 - value
//     SUBTRACT(const std::string& var1, const std::string& var2, uint16_t value)
//         : resultVar(var1), operand1Var(var2), operand2Var(""), 
//           operand1Value(0), operand2Value(value), operand1IsVar(true), operand2IsVar(false) {}
    
//     // var1 = value - var3
//     SUBTRACT(const std::string& var1, uint16_t value, const std::string& var3)
//         : resultVar(var1), operand1Var(""), operand2Var(var3), 
//           operand1Value(value), operand2Value(0), operand1IsVar(false), operand2IsVar(true) {}
    
//     // var1 = value1 - value2
//     SUBTRACT(const std::string& var1, uint16_t value1, uint16_t value2)
//         : resultVar(var1), operand1Var(""), operand2Var(""), 
//           operand1Value(value1), operand2Value(value2), operand1IsVar(false), operand2IsVar(false) {}

//     void execute(Process& process) override {
//         uint16_t op1 = operand1IsVar ? process.getVariable(operand1Var) : operand1Value;
//         uint16_t op2 = operand2IsVar ? process.getVariable(operand2Var) : operand2Value;
        
//         // Perform subtraction with underflow protection
//         uint16_t result = (op1 >= op2) ? (op1 - op2) : 0;
        
//         process.setVariable(resultVar, result);
        
//         std::cout << "[Process " << process.getProcessName() << "] " 
//                   << resultVar << " = " << op1 << " - " << op2 << " = " << result << std::endl;
//     }
// };

// class SLEEP : public ICommand {
// private:
//     uint8_t cpuTicks;

// public:
//     SLEEP(uint8_t ticks) : cpuTicks(ticks) {}

//     void execute(Process& process) override {
//         std::cout << "[Process " << process.getProcessName() << "] " 
//                   << "Sleeping for " << static_cast<int>(cpuTicks) << " CPU ticks" << std::endl;
        
//         // Sleep for the specified number of ticks
//         // Assuming each tick is a small time unit (e.g., 10ms)
//         std::this_thread::sleep_for(std::chrono::milliseconds(cpuTicks * 10));
        
//         std::cout << "[Process " << process.getProcessName() << "] " 
//                   << "Woke up from sleep" << std::endl;
//     }
// };

// class FOR : public ICommand {
// private:
//     std::vector<std::unique_ptr<ICommand>> instructions;
//     uint8_t repeatCount;

// public:
//     FOR(std::vector<std::unique_ptr<ICommand>>&& instrs, uint8_t repeats)
//         : instructions(std::move(instrs)), repeatCount(repeats) {}

//     void execute(Process& process) override {
//         std::cout << "[Process " << process.getProcessName() << "] " 
//                   << "Starting FOR loop (" << static_cast<int>(repeatCount) << " iterations)" << std::endl;
        
//         for (uint8_t i = 0; i < repeatCount; ++i) {
//             std::cout << "[Process " << process.getProcessName() << "] " 
//                       << "Loop iteration " << static_cast<int>(i + 1) << "/" 
//                       << static_cast<int>(repeatCount) << std::endl;
            
//             // Execute all instructions in the loop
//             for (auto& instruction : instructions) {
//                 instruction->execute(process);
//             }
//         }
        
//         std::cout << "[Process " << process.getProcessName() << "] " 
//                   << "FOR loop completed" << std::endl;
//     }
// };


#include "ICommand.h"
#include "process.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>

// ----- PRINT -----
PRINT::PRINT() : message(""), variableName(""), isVariable(false) {}

PRINT::PRINT(const std::string& varName) : message(""), variableName(varName), isVariable(true) {}

PRINT::PRINT(const std::string& msg, bool isMsg) : message(msg), variableName(""), isVariable(false) {}

void PRINT::execute(Process& process) {
    if (isVariable && !variableName.empty()) {
        uint16_t value = process.getVariable(variableName);
        std::cout << "[Process " << process.getProcessName() << "] " 
                  << variableName << " = " << value << std::endl;
    } else if (!message.empty()) {
        std::cout << "[Process " << process.getProcessName() << "] " 
                  << message << std::endl;
    } else {
        std::cout << "[Process " << process.getProcessName() << "] " 
                  << "Hello world from " << process.getProcessName() << std::endl;
    }
}

// ----- DECLARE -----
DECLARE::DECLARE(const std::string& varName, uint16_t val)
    : variableName(varName), value(val) {}

void DECLARE::execute(Process& process) {
    process.setVariable(variableName, value);
    std::cout << "[Process " << process.getProcessName() << "] " 
              << "Declared " << variableName << " = " << value << std::endl;
}

// ----- ADD -----
ADD::ADD(const std::string& var1, const std::string& var2, const std::string& var3)
    : resultVar(var1), operand1Var(var2), operand2Var(var3),
      operand1Value(0), operand2Value(0), operand1IsVar(true), operand2IsVar(true) {}

ADD::ADD(const std::string& var1, const std::string& var2, uint16_t value)
    : resultVar(var1), operand1Var(var2), operand2Var(""),
      operand1Value(0), operand2Value(value), operand1IsVar(true), operand2IsVar(false) {}

ADD::ADD(const std::string& var1, uint16_t value, const std::string& var3)
    : resultVar(var1), operand1Var(""), operand2Var(var3),
      operand1Value(value), operand2Value(0), operand1IsVar(false), operand2IsVar(true) {}

ADD::ADD(const std::string& var1, uint16_t value1, uint16_t value2)
    : resultVar(var1), operand1Var(""), operand2Var(""),
      operand1Value(value1), operand2Value(value2), operand1IsVar(false), operand2IsVar(false) {}

void ADD::execute(Process& process) {
    uint16_t op1 = operand1IsVar ? process.getVariable(operand1Var) : operand1Value;
    uint16_t op2 = operand2IsVar ? process.getVariable(operand2Var) : operand2Value;

    uint32_t result = static_cast<uint32_t>(op1) + static_cast<uint32_t>(op2);
    uint16_t finalResult = std::min(result, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()));

    process.setVariable(resultVar, finalResult);

    std::cout << "[Process " << process.getProcessName() << "] " 
              << resultVar << " = " << op1 << " + " << op2 << " = " << finalResult << std::endl;
}

// ----- SUBTRACT -----
SUBTRACT::SUBTRACT(const std::string& var1, const std::string& var2, const std::string& var3)
    : resultVar(var1), operand1Var(var2), operand2Var(var3),
      operand1Value(0), operand2Value(0), operand1IsVar(true), operand2IsVar(true) {}

SUBTRACT::SUBTRACT(const std::string& var1, const std::string& var2, uint16_t value)
    : resultVar(var1), operand1Var(var2), operand2Var(""),
      operand1Value(0), operand2Value(value), operand1IsVar(true), operand2IsVar(false) {}

SUBTRACT::SUBTRACT(const std::string& var1, uint16_t value, const std::string& var3)
    : resultVar(var1), operand1Var(""), operand2Var(var3),
      operand1Value(value), operand2Value(0), operand1IsVar(false), operand2IsVar(true) {}

SUBTRACT::SUBTRACT(const std::string& var1, uint16_t value1, uint16_t value2)
    : resultVar(var1), operand1Var(""), operand2Var(""),
      operand1Value(value1), operand2Value(value2), operand1IsVar(false), operand2IsVar(false) {}

void SUBTRACT::execute(Process& process) {
    uint16_t op1 = operand1IsVar ? process.getVariable(operand1Var) : operand1Value;
    uint16_t op2 = operand2IsVar ? process.getVariable(operand2Var) : operand2Value;

    uint16_t result = (op1 >= op2) ? (op1 - op2) : 0;

    process.setVariable(resultVar, result);

    std::cout << "[Process " << process.getProcessName() << "] " 
              << resultVar << " = " << op1 << " - " << op2 << " = " << result << std::endl;
}

// ----- SLEEP -----
SLEEP::SLEEP(uint8_t ticks) : cpuTicks(ticks) {}

void SLEEP::execute(Process& process) {
    std::cout << "[Process " << process.getProcessName() << "] " 
              << "Sleeping for " << static_cast<int>(cpuTicks) << " CPU ticks" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(cpuTicks * 10));

    std::cout << "[Process " << process.getProcessName() << "] " 
              << "Woke up from sleep" << std::endl;
}

// ----- FOR -----
FOR::FOR(std::vector<std::unique_ptr<ICommand>>&& instrs, uint8_t repeats)
    : instructions(std::move(instrs)), repeatCount(repeats) {}

void FOR::execute(Process& process) {
    std::cout << "[Process " << process.getProcessName() << "] "
              << "Starting FOR loop (" << static_cast<int>(repeatCount) << " iterations)" << std::endl;

    for (uint8_t i = 0; i < repeatCount; ++i) {
        std::cout << "[Process " << process.getProcessName() << "] "
                  << "Loop iteration " << static_cast<int>(i + 1) << "/" << static_cast<int>(repeatCount) << std::endl;

        for (auto& instruction : instructions) {
            instruction->execute(process);
        }
    }

    std::cout << "[Process " << process.getProcessName() << "] "
              << "FOR loop completed" << std::endl;
}

//to catch errors
//-andrei
UNKNOWN::UNKNOWN() 
    : reason("No specific reason provided.") {}

UNKNOWN::UNKNOWN(const std::string& reasonMessage) 
    : reason(reasonMessage) {}

void UNKNOWN::execute(Process& process) {
    std::cerr << "[Process " << process.getProcessName() << "] "
              << "ERROR: Unknown command encountered. Reason: " << reason << std::endl;
}
