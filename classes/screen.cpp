#include "screen.h"

ScreenSession::ScreenSession(const std::string& n,
                             int curr,
                             int total,
                             const std::string& time)
  : name(n),
    current_line(curr),
    total_lines(total),
    timestamp(time),
    finished(false),
    next(nullptr)
{}