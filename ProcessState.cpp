#include "ProcessState.h" 

std::string processStateToString(ProcessState state) {
    switch (state) {
        case ProcessState::NONE: return "NONE";
        case ProcessState::WAITING: return "WAITING";
        case ProcessState::RUNNING: return "RUNNING";
        case ProcessState::FINISHED: return "FINISHED";
        default: return "UNKNOWN";
    }
}