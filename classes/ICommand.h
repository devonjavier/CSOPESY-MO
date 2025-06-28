#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

class Process; // Forward declaration to avoid circular include

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute(Process& process) = 0;
};

// ========== Concrete Commands ========== //

class PRINT : public ICommand {
private:
    std::string message;
    std::string variableName;
    bool isVariable;

public:
    PRINT(); // default constructor
    PRINT(const std::string& varName); // print variable
    PRINT(const std::string& msg, bool isMsg); // print custom message

    void execute(Process& process) override;
};

class DECLARE : public ICommand {
private:
    std::string variableName;
    uint16_t value;

public:
    DECLARE(const std::string& varName, uint16_t val);
    void execute(Process& process) override;
};

class ADD : public ICommand {
private:
    std::string resultVar;
    std::string operand1Var;
    std::string operand2Var;
    uint16_t operand1Value;
    uint16_t operand2Value;
    bool operand1IsVar;
    bool operand2IsVar;

public:
    ADD(const std::string& var1, const std::string& var2, const std::string& var3);
    ADD(const std::string& var1, const std::string& var2, uint16_t value);
    ADD(const std::string& var1, uint16_t value, const std::string& var3);
    ADD(const std::string& var1, uint16_t value1, uint16_t value2);
    void execute(Process& process) override;
};

class SUBTRACT : public ICommand {
private:
    std::string resultVar;
    std::string operand1Var;
    std::string operand2Var;
    uint16_t operand1Value;
    uint16_t operand2Value;
    bool operand1IsVar;
    bool operand2IsVar;

public:
    SUBTRACT(const std::string& var1, const std::string& var2, const std::string& var3);
    SUBTRACT(const std::string& var1, const std::string& var2, uint16_t value);
    SUBTRACT(const std::string& var1, uint16_t value, const std::string& var3);
    SUBTRACT(const std::string& var1, uint16_t value1, uint16_t value2);
    void execute(Process& process) override;
};

class SLEEP : public ICommand {
private:
    uint8_t cpuTicks;

public:
    SLEEP(uint8_t ticks);
    void execute(Process& process) override;
};

class FOR : public ICommand {
private:
    std::vector<std::unique_ptr<ICommand>> instructions;
    uint8_t repeatCount;

public:
    FOR(std::vector<std::unique_ptr<ICommand>>&& instrs, uint8_t repeats);
    void execute(Process& process) override;
};

class UNKNOWN : public ICommand {
private:
    std::string reason;

public:
    UNKNOWN(); // Default constructor
    UNKNOWN(const std::string& reasonMessage);
    void execute(Process& process) override;
};
