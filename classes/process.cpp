#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include "classes/ICommand.cpp"

enum class ProcessState{
    IDLE,
    WAITING,
    RUNNING,
    FINISHED
};

std::string processStateToString(ProcessState state) { 
    switch(state) {
        case ProcessState::IDLE:
            return "IDLE";
        case ProcessState::WAITING:
            return "WAITING";
        case ProcessState::RUNNING:
            return "RUNNING";
        case ProcessState::FINISHED:
            return "FINISHED";
        default: return "UNKNOWN";
    }
}

class Process {
    private:
        int pid;
        std::string process_name;
        std::vector<ICommand*> instructions;
        std::chrono::time_point<std::chrono::system_clock> start_time;
        std::chrono::time_point<std::chrono::system_clock> end_time;
        int current_core_id;
        std::vector<ICommand*> instructions;
        ProcessState state;


    public:
        Process(int id, const std::string& name, int burst)
            : pid(id), process_name(name), 
            // burst_time(burst), 
            //   remaining_burst_time(burst), waiting_time(0), turnaround_time(0),
              current_core_id(-1), state(ProcessState::IDLE) {}


        //TODO / To think about lmfao
            //1. need setStartTime / setEndTime? for

        void addInstruction(ICommand* instruction) {
            instructions.push_back(instruction);
        }

        const std::vector<ICommand*>& getInstructions() const {
            return instructions;
        }

        void runInstructions() {
            for (ICommand* cmd : instructions) {
                cmd->execute();
            }
        }




        int getPid() const {
            return pid;
        }
        std::string getProcessName() const {
            return process_name;
        }
        // int getBurstTime() const {
        //     return burst_time;
        // }
        // int getRemainingBurstTime() const {
        //     return remaining_burst_time;
        // }
        // int getWaitingTime() const {
        //     return waiting_time;
        // }
        // int getTurnaroundTime() const {
        //     return turnaround_time;
        // }
        std::chrono::time_point<std::chrono::system_clock> getStartTime() const {
            return start_time;
        }
        std::chrono::time_point<std::chrono::system_clock> getEndTime() const {
            return end_time;
        }
        int getCurrentCoreId() const {
            return current_core_id;
        }
        ProcessState getState() const {
            return state;
        }

        // std::string getFormattedStartDate() const {
        //     struct tm temp;

        //     #ifdef _WIN32
        //         if (localtime_s(&temp, &process_start_date) != 0) {
        //             return "Error formatting time (Windows)"; //error message
        //         }
        //     #else //macOS for andrei
        //         struct tm* localTm = localtime(&process_start_date);
        //         if (localTm == nullptr) {
        //             return "Error formatting time (Unix)";
        //         }
        //         temp = *localTm;
        //     #endif

        //     std::ostringstream oss;
        //     // %d: Day, %m: Month, %Y: Year, %I: Hour, %M: Minute, %S: Second, %p: AM/PM indicator
        //     oss << std::put_time(&temp, "(%d/%m/%Y %I:%M:%S%p)");
        //     return oss.str(); //return formatted date
        // }

        // void setState(ProcessState newState, int coreId = -1) {
        // void setState(ProcessState newState, int coreId) {
        void setState(ProcessState newState) {
            this->state = newState;
            // this->current_core_id = coreId;
            if (newState == ProcessState::RUNNING && //coreId != -1 &&
                this->start_time.time_since_epoch().count() == 0) { // Check if start_time is uninitialized
                this->start_time = std::chrono::system_clock::now();
            }
        }

        std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp) {
            if (tp.time_since_epoch().count() == 0) return "N/A";
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            char buffer[100];
            std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %I:%M:%S%p", std::localtime(&tt));
            return std::string(buffer);
        }

        // void displayProcess() const {
        //     // std::cout << "Process ID: " << pid << "\n"
        //     //           << "Process Name: " << process_name << "\n"
        //     //           << "Priority: " << priority << "\n"
        //     //           << "Burst Time: " << burst_time << "\n"
        //     //           << "Waiting Time: " << waiting_time << "\n"
        //     //           << "Turnaround Time: " << turnaround_time 
        //     //           << "Process Start Date: " << getFormattedStartDate() << "\n"
        //     //           << "Current Core ID: " << current_core_id << "\n"
        //     //           << std::endl;
        //     std::cout << "Process ID: " << pid << "\n"
        //               << "Process Name: " << process_name << "\n"
        //               << "Priority: " << priority << "\n"
        //               << "Burst Time: " << burst_time << "\n"
        //               << "Remaining Burst Time: " << remaining_burst_time << "\n"
        //               << "Waiting Time: " << waiting_time << "\n"
        //               << "Turnaround Time: " << turnaround_time << "\n"
        //             //   << "Process Start Date: " << start_time << "\n"
        //               << "Current Core ID: " << current_core_id << "\n"
        //               << "State: " << static_cast<int>(state) // Convert enum to int for display
        //               << std::endl;
        // } 


        // clean up step

        ~Process() {
            for (ICommand* instruction : instructions) {
                delete instruction;
            }
        }
};