#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include <chrono>

class ScreenSession {
public:
    ScreenSession(const std::string& name,
                  int current_line,
                  int total_lines,
                  const std::string& timestamp);

    // data members
    std::string name;
    int current_line;
    int total_lines;
    std::string timestamp;
    bool finished;            // tracks whether done
    ScreenSession* next;      // for chaining

    // (any other member functions you have)
};

#endif // SCREEN_H
