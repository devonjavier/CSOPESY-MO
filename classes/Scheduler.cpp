#include "process.cpp"
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class Scheduler { 

    private:
    std::vector<Process> processes;
    std::queue<Process> ready_queue; 
    std::vector<Process> runningProcesses;
    std::vector<Process> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> SchedulerRunning{false};
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::string SchedulerType; 
    int quantumCycles; 

    public:
    Scheduler(const std::string& Scheduler, int quantum) 
        : SchedulerType(Scheduler), quantumCycles(quantum) {}

    void addProcess(const Process& process) {
        ready_queue.push(process);
    }

    void queueProcesses() {
        std::unique_lock<std::mutex> lock(queueMutex);

        for (auto& process : processes) {
            // If the process is not yet queued or completed, add to ready queue
            if (process.getState() == ProcessState::IDLE) {
                process.setState(ProcessState::WAITING);
                ready_queue.push(process);
                queueCV.notify_one(); 
            }
        }
    }

    void startScheduler(int num_cpu) {
        SchedulerRunning = true;

        // Create worker threads based on the number of CPU cores
        for (int i = 0; i < num_cpu ; ++i) {
            workerThreads.emplace_back(&Scheduler::SchedulerLoop, this);
        }
    }


    bool isSchedulerRunning() const {
        return SchedulerRunning;
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

