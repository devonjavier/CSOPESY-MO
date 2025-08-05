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
    virtual std::string toString(const Process& process) const = 0; 
};

// ========== Concrete Commands ========== //

class PRINT : public ICommand {
private:
    std::string message;
    std::string variableName;
    bool isVariable;
    bool isCombined = false; 

public:
    PRINT(); // default constructor
    PRINT(const std::string& varName); // print variable
    PRINT(const std::string& msg, bool isMsg); // print custom message
    PRINT(const std::string& msg, const std::string& varName);

    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
};

class DECLARE : public ICommand {
private:
    std::string variableName;
    uint16_t value;

public:
    DECLARE(const std::string& varName, uint16_t val);
    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
    int getRequiredPage(size_t page_size);
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
    static int getRequiredPage(size_t page_size) { return 0; }
    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
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
    static int getRequiredPage(size_t page_size) { return 0; }
    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
};

class SLEEP : public ICommand {
private:
    uint8_t cpuTicks;

public:
    SLEEP(uint8_t ticks);
    void execute(Process& process) override;
    std::string toString(const Process& process) const;
};

class FOR : public ICommand {
private:
    std::vector<std::unique_ptr<ICommand>> instructions;
    uint8_t repeatCount;

public:
    FOR(std::vector<std::unique_ptr<ICommand>>&& instrs, uint8_t repeats);
    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
};

class READ : public ICommand {
private:
    std::string variable_name;
    uint32_t memory_address; 

public:
    READ(const std::string& var, uint32_t address);
    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
    uint32_t getAddress() const { return memory_address; }
    // NEW: A static helper to determine which page this command accesses.
    int getRequiredPage(size_t page_size) const;
};


class WRITE : public ICommand {
private:
    std::string variable_name; // The variable WHOSE VALUE we are writing
    uint32_t memory_address;

public:
    WRITE(const std::string& var, uint32_t address);
    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
    uint32_t getAddress() const { return memory_address; }
    // NEW: A static helper to determine which page this command accesses.
    int getRequiredPage(size_t page_size) const;
};

class UNKNOWN : public ICommand {
private:
    std::string reason;

public:
    UNKNOWN(); // Default constructor
    UNKNOWN(const std::string& reasonMessage);
    void execute(Process& process) override;
    std::string toString(const Process& process) const override;
};

