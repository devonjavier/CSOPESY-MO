#include "process.cpp"
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>

class Scheduler { 

    private:
    std::vector<Process> processes;
    std::queue<Process> ready_queue; 
    std::vector<Process> runningProcesses;
    std::vector<Process> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> SchedulerRunning{false};
    std::atomic<bool> GeneratingProcesses{false};
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::string SchedulerType; 
    int quantumCycles; 

    public:
    Scheduler(const std::string& Scheduler, int quantum) 
        : SchedulerType(Scheduler), quantumCycles(quantum) {}

    void addProcess(const Process& process) {
        std::lock_guard<std::mutex> lock(queueMutex);
        processes.push_back(process);
    }

    void queueProcesses() {
        std::lock_guard<std::mutex> lock(queueMutex);
        
        for (auto& process : processes) {
            if (process.getState() == ProcessState::IDLE) {
                process.setState(ProcessState::WAITING);
                ready_queue.push(process);
                queueCV.notify_one(); 
            }
        }
    }

    void schedulerAlgo(int coreId) {
        while (true) {
            Process current;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCV.wait(lock, [this] {
                    return !ready_queue.empty() || !SchedulerRunning || !GeneratingProcesses;
                });

                if (!SchedulerRunning && ready_queue.empty()) break;
                if (ready_queue.empty()) continue;

                current = ready_queue.front();
                ready_queue.pop();

                current.setState(ProcessState::RUNNING);
                current.setCurrentCoreId(coreId); // ✅ Set here

                runningProcesses.push_back(current);
            }

            // current.runInstructions();
            current.setState(ProcessState::FINISHED);

            {
                std::lock_guard<std::mutex> lock(queueMutex);

                auto it = std::find_if(runningProcesses.begin(), runningProcesses.end(),
                                    [&](const Process& p) { return p.getPid() == current.getPid(); });
                if (it != runningProcesses.end()) {
                    runningProcesses.erase(it);
                }

                completedProcesses.push_back(current);
            }

            checkIfComplete();
        }

        std::cout << "Worker thread for core " << coreId << " exiting.\n";
    }




    void startScheduler(int num_cpu) {
        SchedulerRunning = true;
        GeneratingProcesses = true;
        for (int i = 0; i < num_cpu ; ++i) {
            workerThreads.emplace_back(&Scheduler::schedulerAlgo, this);
        }
    }

    void stopScheduler() {
        GeneratingProcesses = false;
        std::cout << "Scheduler stop signal received. Waiting for all processes to finish...\n";
    }

    void finalizeScheduler() {
    SchedulerRunning = false;
        for (auto& t : workerThreads) {
            if (t.joinable())
                t.join();
        }
        std::cout << "Scheduler fully shut down. All processes completed.\n";
    }


    bool isSchedulerRunning() const {
        return SchedulerRunning;
    }

    bool isGeneratingProcesses() const {
        return GeneratingProcesses;
    }

    bool runningThreadsDone() {
        std::lock_guard<std::mutex> lock(queueMutex);
        return std::all_of(runningProcesses.begin(), runningProcesses.end(), [](const Process& p) {
            return p.getState() == ProcessState::FINISHED;
        });
    }


    void checkIfComplete() {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!GeneratingProcesses && ready_queue.empty() && runningThreadsDone()) {
            SchedulerRunning = false;
            queueCV.notify_all();
            std::cout << "All processes completed. Scheduler is shutting down.\n";
        }
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

