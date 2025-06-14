#ifndef PROCESS_STATE_H
#define PROCESS_STATE_H

#include <string> 

enum class ProcessState {
    NONE,
    WAITING,
    RUNNING,
    FINISHED
};

std::string processStateToString(ProcessState state);// {

#endif