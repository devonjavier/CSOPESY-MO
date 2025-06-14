#include <string>
#include <chrono>
#include <iostream>

class ScreenSession {
public:
    std::string name;
    int current_line;
    int total_lines;
    std::string timestamp;
    bool finished = false; // New flag to track if the session is done
    ScreenSession* next;

    ScreenSession(std::string n, int curr, int total, std::string time)
        : name(n), current_line(curr), total_lines(total), timestamp(time), next(nullptr) {}
};

