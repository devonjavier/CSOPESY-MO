#include "ICommand.h"
#include "process.h"
#include "helper.cpp" // becomes global
#include <iostream>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include <sstream>

// ----- PRINT -----
PRINT::PRINT() : message(""), variableName(""), isVariable(false) {}

PRINT::PRINT(const std::string& varName) : message(""), variableName(varName), isVariable(true) {}

PRINT::PRINT(const std::string& msg, bool isMsg) : message(msg), variableName(""), isVariable(false) {}

void PRINT::execute(Process& process) {

    std::string messageContent;
    if (isVariable && !variableName.empty()) {
        uint16_t value = process.getVariableValue(variableName);
        messageContent = variableName + " = " + std::to_string(value);
    } else if (!message.empty()) {
        messageContent = message;
    } else {
        messageContent = "Hello world from " + process.getProcessName();
    }


    std::string log = "[Process " + process.getProcessName() + "] " + get_timestamp() + " Core ID: " + 
        std::to_string(process.getCurrentCoreId()) + ", " + messageContent;
    
    process.addLog(log);
}

std::string PRINT::toString(const Process& process) const {
    if (isVariable) {
        return "PRINT variable \"" + variableName + "\"";
    } else {
        return "PRINT message \"" + message + "\"";
    }
}

// ----- DECLARE -----
DECLARE::DECLARE(const std::string& varName, uint16_t val)
    : variableName(varName), value(val) {}

void DECLARE::execute(Process& process) {
    process.setVariable(variableName, value);
    std::string log = "[Process " + process.getProcessName() + "] " + get_timestamp() + " Core ID: " + std::to_string(process.getCurrentCoreId()) + ", "
                    + "Declared " + variableName + " = " + std::to_string(value);
    process.addLog(log);
}

int DECLARE::getRequiredPage(size_t page_size) {
    return 0; 
}

std::string DECLARE::toString(const Process& process) const {
    return "DECLARE " + variableName + " = " + std::to_string(value);
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
    uint16_t op1 = operand1IsVar ? process.getVariableValue(operand1Var) : operand1Value;
    uint16_t op2 = operand2IsVar ? process.getVariableValue(operand2Var) : operand2Value;

    uint32_t result = static_cast<uint32_t>(op1) + static_cast<uint32_t>(op2);
    uint16_t finalResult = std::min(result, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()));

    process.setVariable(resultVar, finalResult);

    std::string log = "[Process " + process.getProcessName() + "] " + get_timestamp() + " Core ID: " + 
                    std::to_string(process.getCurrentCoreId()) + ", "
                    + resultVar + " = " + std::to_string(op1) 
                    + " + " + std::to_string(op2) 
                    + " = " + std::to_string(finalResult);
    
    process.addLog(log);
}

std::string ADD::toString(const Process& process) const {
    auto a = operand1IsVar ? operand1Var : std::to_string(operand1Value);
    auto b = operand2IsVar ? operand2Var : std::to_string(operand2Value);
    return "ADD " + resultVar + " = " + a + " + " + b;
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
    uint16_t op1 = operand1IsVar ? process.getVariableValue(operand1Var) : operand1Value;
    uint16_t op2 = operand2IsVar ? process.getVariableValue(operand2Var) : operand2Value;

    uint16_t result = (op1 >= op2) ? (op1 - op2) : 0;

    process.setVariable(resultVar, result);

    std::string log = "[Process " + process.getProcessName() + "] " 
                    + get_timestamp() + " Core ID: " + std::to_string(process.getCurrentCoreId()) + ", "
                    + resultVar + " = " + std::to_string(op1) 
                    + " - " + std::to_string(op2) 
                    + " = " + std::to_string(result);

    process.addLog(log);
}

std::string SUBTRACT::toString(const Process& process) const {
    auto a = operand1IsVar ? operand1Var : std::to_string(operand1Value);
    auto b = operand2IsVar ? operand2Var : std::to_string(operand2Value);
    return "SUBTRACT " + resultVar + " = " + a + " - " + b;
}

// ----- SLEEP -----
SLEEP::SLEEP(uint8_t ticks) : cpuTicks(ticks) {}

void SLEEP::execute(Process& process) {
    std::string startLog = "[Process " + process.getProcessName() + "] " + get_timestamp() + " Core ID: " + 
        std::to_string(process.getCurrentCoreId()) + ", " + " Sleeping for " + std::to_string(cpuTicks) + " CPU ticks";
    process.addLog(startLog);

    std::this_thread::sleep_for(std::chrono::milliseconds(cpuTicks * 10));

    std::string endLog = "[Process " + process.getProcessName() + "] " + get_timestamp() + " Core ID: " + 
        std::to_string(process.getCurrentCoreId()) + "Woke up from sleep";
    process.addLog(endLog);
}

std::string SLEEP::toString(const Process& process) const {
    return "SLEEP " + std::to_string(cpuTicks) + " ticks";
}

// ----- FOR -----
FOR::FOR(std::vector<std::unique_ptr<ICommand>>&& instrs, uint8_t repeats)
    : instructions(std::move(instrs)), repeatCount(repeats) {}

void FOR::execute(Process& process) {
    std::string startLog = "[Process " + process.getProcessName() + "] " + get_timestamp() + " Core ID: " + 
        std::to_string(process.getCurrentCoreId()) + ", " + "Starting FOR loop (" + std::to_string(repeatCount) + " iterations)";
    process.addLog(startLog);
    for (uint8_t i = 0; i < repeatCount; ++i) {
        // std::cout << "[Process " << process.getProcessName() << "] "
        //           << "Loop iteration " << static_cast<int>(i + 1) << "/" << static_cast<int>(repeatCount) << std::endl;

        for (auto& instruction : instructions) {
            instruction->execute(process);
        }
    }

    std::string endLog = "[Process " + process.getProcessName() + "] " + get_timestamp() + " Core ID: " + 
        std::to_string(process.getCurrentCoreId()) + ", " + "FOR loop completed";
    process.addLog(endLog);
}

std::string FOR::toString(const Process& process) const {
    std::string instrsStr;
    instrsStr.reserve(instructions.size() * 20);  // optional: avoid a few reallocs

    for (const auto& instr : instructions) {
        instrsStr += instr->toString(process);
        instrsStr += "; ";
    }
    if (!instrsStr.empty()) {
        // remove the last “; ”
        instrsStr.erase(instrsStr.end() - 2, instrsStr.end());
    }

    return "FOR " 
         + std::to_string(repeatCount) 
         + " times: [" 
         + instrsStr 
         + "]";
}

READ::READ(const std::string& var, uint32_t address)
    : variable_name(var), memory_address(address) {}

void READ::execute(Process& process) {
    // 1. ACCESS VIOLATION CHECK: Is the address within the process's allocated memory?
    if (memory_address >= process.getMemorySize()) {
        std::stringstream ss;
        ss << "Access violation at 0x" << std::hex << memory_address << ".";
        process.terminate(ss.str()); // Terminate the process
        return; // Stop execution immediately
    }

    // 2. Perform the read. For this simulation, we read a default value.
    // In a more complex simulation, you would read from a memory buffer in the Process class.
    // As per the spec, an uninitialized read returns 0.
    uint16_t value_read = process.readMemory(memory_address);
    
    // 3. Store the result in the process's symbol table.
    process.setVariable(variable_name, value_read);
}

std::string READ::toString(const Process& process) const {
    std::stringstream ss;
    ss << "READ " << variable_name << " 0x" << std::hex << memory_address;
    return ss.str();
}

// Helper for the scheduler to do pre-execution checks

WRITE::WRITE(const std::string& var, uint32_t address)
    : variable_name(var), memory_address(address) {}

void WRITE::execute(Process& process) {
    // 1. ACCESS VIOLATION CHECK:
    if (memory_address >= process.getMemorySize()) {
        std::stringstream ss;
        ss << "Access violation at 0x" << std::hex << memory_address << ".";
        process.terminate(ss.str());
        return;
    }

    // 2. Get the value to write from the process's symbol table.
    uint16_t value_to_write = 0; // Default to 0 if variable doesn't exist
    process.getVariable(variable_name, value_to_write);

    // 3. Perform the write.
    // In this simulation, this is a conceptual operation. We can log it.
    // In a more complex sim, you would write to a memory buffer.
    process.writeMemory(memory_address, value_to_write);
    
    // 4. Mark the page as "dirty" since it has been modified.
    int page_number = getRequiredPage(process.getPageTable()->getPageSize());
    process.getPageTable()->setDirty(page_number, true);
}

std::string WRITE::toString(const Process& process) const {
    uint16_t value = 0;
    process.getVariable(variable_name, value);
    std::stringstream ss;
    ss << "WRITE 0x" << std::hex << memory_address << " (value of " << variable_name << " = " << std::dec << value << ")";
    return ss.str();
}

int READ::getRequiredPage(size_t page_size) const {
    return this->memory_address / page_size;
}

int WRITE::getRequiredPage(size_t page_size) const {
    return this->memory_address / page_size;
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

std::string UNKNOWN::toString(const Process& process) const {
    return "UNKNOWN command: " + reason;
}

