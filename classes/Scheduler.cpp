#include "classes/Process.cpp"
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

    void SchedulerLoop() {
    while (SchedulerRunning) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCV.wait(lock, [this]{ return !ready_queue.empty() || !SchedulerRunning; });
        
        if (!ready_queue.empty()) {
            Process proc = ready_queue.front();
            ready_queue.pop();
            lock.unlock();
            
            // proc.execute()
        }
    }
}

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

    
        for (int i = 0; i < num_cpu ; ++i) {
            workerThreads.emplace_back(&Scheduler::SchedulerLoop, this);
        }
    }


    bool isSchedulerRunning() const {
        return SchedulerRunning;
    }


};

