#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

// Forward declaration for Process if needed
// class Process;

class ICommand {
    public:
        virtual ~ICommand() = default;

        virtual void execute(std::unordered_map<std::string, uint16_t>& variables) = 0;
};


class PRINT : public ICommand {};
class DECLARE : public ICommand {};
class ADD : public ICommand {};
class SUBTRACT : public ICommand {};
class SLEEP : public ICommand {};
class FOR : public ICommand {};
