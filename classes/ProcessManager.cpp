#include "process.cpp"
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class ProcessManager { 

    private:
    std::queue<Process> processQueue; 
    std::vector<Process> runningProcesses;
    std::vector<Process> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> schedulerRunning{false};
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::string schedulerType; 
    int quantumCycles; 

    public:
    ProcessManager(const std::string& scheduler, int quantum) 
        : schedulerType(scheduler), quantumCycles(quantum) {}

    void addProcess(const Process& process) {
        processQueue.push(process);
    }

    // void FCFS() {
    //     while (!processQueue.empty()) {
    //         Process currentProcess = processQueue.front();
    //         processQueue.pop();
    //         currentProcess.setState(ProcessState::RUNNING);
    //         runningProcesses.push_back(currentProcess);

    //         // Simulate process execution
    //         std::this_thread::sleep_for(std::chrono::milliseconds(currentProcess.getBurstTime()));
    //         currentProcess.setState(ProcessState::COMPLETED);
    //         completedProcesses.push_back(currentProcess);
    //     }
    // }
};