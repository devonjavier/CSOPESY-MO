#include <string>
#include <chrono>
#include <iostream>

struct ScreenSession {
    std::string name;
    int current_line = 0;
    int total_lines = 0;
    std::string timestamp;
    ScreenSession *next = nullptr;  // linked list

    // constructor
    ScreenSession(std::string n, int current_line, int total_lines, std::string timestamp)
        : name(n), current_line(current_line), total_lines(total_lines), timestamp(timestamp) {}
};

