#pragma once
#include <string>
#include <unordered_map>

class Process {
public:
    std::string getProcessName() const;
    uint16_t getVariable(const std::string& var) const;
    void setVariable(const std::string& var, uint16_t value);
    // You only list what's needed by ICommand here
};