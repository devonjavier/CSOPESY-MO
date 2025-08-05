#include "process.cpp"
#include <queue>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <windows.h>

class Scheduler { 

    private:
    std::vector<std::unique_ptr<Process>> processes;
    std::deque<std::unique_ptr<Process>> ready_queue;
    std::vector<std::unique_ptr<Process>> runningProcesses;
    std::vector<std::unique_ptr<Process>> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> schedulerRunning{false};
    std::atomic<bool> generatingProcesses{false};            //i think we only need one flag right??
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::string SchedulerType;
    int quantumCycles; 
    uint16_t programcounter = 0;

    void fcfs_scheduler(int coreId) {
        while (this->schedulerRunning) {
            std::unique_ptr<Process> current_process;

            {
                std::unique_lock<std::mutex> lock(this->queueMutex);
                this->queueCV.wait(lock, [this] {
                    return !this->ready_queue.empty() || !this->schedulerRunning;
                });

                if (!this->schedulerRunning && this->ready_queue.empty()) {
                    break; // Exit the loop
                }
                if (this->ready_queue.empty()) {
                    continue; // Spurious wakeup, wait again
                }
                
                current_process = std::move(this->ready_queue.front());
                this->ready_queue.pop_front();
            } 

            // process execution
            if (current_process) {
                current_process->setState(ProcessState::RUNNING);
                current_process->setCurrentCoreId(coreId);
                
                // fcfs run all instructions
                current_process->runInstructions(); 
                

                current_process->setState(ProcessState::FINISHED);

                {
                    std::lock_guard<std::mutex> lock(this->queueMutex);
                    this->completedProcesses.push_back(std::move(current_process));
                }
            }
        }
        std::cout << "Core " << coreId << ": Exiting FCFS worker thread." << std::endl;
    }

    void rr_scheduler(int coreId){
          while (this->schedulerRunning) {
            std::unique_ptr<Process> current_process;

            // process dequed from ready queue
            {
                std::unique_lock<std::mutex> lock(this->queueMutex);
                this->queueCV.wait(lock, [this] {
                    return !this->ready_queue.empty() || !this->schedulerRunning;
                });

                if (!this->schedulerRunning && this->ready_queue.empty()) {
                    break;
                }
                if (this->ready_queue.empty()) {
                    continue;
                }

                current_process = std::move(this->ready_queue.front());
                this->ready_queue.pop_front();
            } 

            // time slice execution, rr
            if (current_process) {
                current_process->setState(ProcessState::RUNNING);
                current_process->setCurrentCoreId(coreId);


                unsigned int slice = std::min<unsigned>(
                    current_process->getRemainingBurst(),
                    this->quantumCycles
                ); // range 




                current_process->runInstructionSlice(slice);

                // Update the remaining burst time.
                current_process->setRemainingBurst(
                    current_process->getRemainingBurst() - slice
                );

 
                if (current_process->getRemainingBurst() > 0) {
                    current_process->setState(ProcessState::WAITING);
                    {
                        std::lock_guard<std::mutex> lock(this->queueMutex);
                        this->ready_queue.push_back(std::move(current_process));
                        this->queueCV.notify_one(); 
                    }
                } else {

                    current_process->setState(ProcessState::FINISHED);
                    {
                        std::lock_guard<std::mutex> lock(this->queueMutex);
                        this->completedProcesses.push_back(std::move(current_process));
                    }
                }
            }
        }
        std::cout << "Core " << coreId << ": Exiting Round Robin worker thread." << std::endl;
    }

    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", std::localtime(&now_time));
        return std::string(buffer);
    }


    public:
    Scheduler(const std::string& Scheduler, int quantum) 
        : SchedulerType(Scheduler), quantumCycles(quantum) {}

     Process* findProcessByName(const std::string& name) {
        std::lock_guard<std::mutex> lock(queueMutex);
        
        for (const auto& p : processes) { if (p->getProcessName() == name) return p.get(); }
        for (const auto& p : runningProcesses) { if (p->getProcessName() == name) return p.get(); }
        for (const auto& p : completedProcesses) { if (p->getProcessName() == name) return p.get(); }
        

        for (const auto& p : ready_queue) {
            if (p->getProcessName() == name) return p.get();
        }

        return nullptr; 
    }

    std::vector<Process*> getAllProcesses() {
        std::vector<Process*> all_procs;
        std::lock_guard<std::mutex> lock(queueMutex);

        for(const auto& p : processes) { all_procs.push_back(p.get()); }
        for(const auto& p : runningProcesses) { all_procs.push_back(p.get()); }
        for(const auto& p : completedProcesses) { all_procs.push_back(p.get()); }
        
        for(const auto& p : ready_queue) {
            all_procs.push_back(p.get());
        }
        
        return all_procs;
    }


    void checkIfComplete() {
        std::lock_guard<std::mutex> lock(queueMutex);
        // running threads done? 
        if (!generatingProcesses && ready_queue.empty() ) {
            schedulerRunning = false;
            queueCV.notify_all();
            std::cout << "All processes completed. Scheduler is shutting down.\n";
        }
    }
    
    void addProcess(std::unique_ptr<Process> process) {
        std::lock_guard<std::mutex> lock(queueMutex);
        processes.push_back(std::move(process));
    }

    void schedulerAlgo(int coreId) {
        if (this->SchedulerType == "fcfs") {
            fcfs_scheduler(coreId);
        } else if (this->SchedulerType == "rr") {
            rr_scheduler(coreId);
        } else {
            // Handle error case: unknown scheduler type
            std::cerr << "Core " << coreId << ": Unknown scheduler type '" << this->SchedulerType << "'. Exiting thread." << std::endl;
        }
    }

    void queueProcesses() {
        std::lock_guard<std::mutex> lock(queueMutex);

        auto is_idle = [&](std::unique_ptr<Process>& p) {
            if (p && p->getState() == ProcessState::IDLE) {
                p->setState(ProcessState::WAITING);

                ready_queue.push_back(std::move(p)); 
                queueCV.notify_one();

                return true; 
            }
            return false;
        };

        auto new_end = std::remove_if(processes.begin(), processes.end(), is_idle);
        processes.erase(new_end, processes.end());
    }

    // //version 1
    // void schedulerAlgo(int coreId) {
    //     while (true) {
    //         Process current;
    //         {
    //             //dequeue
    //             std::unique_lock<std::mutex> lock(queueMutex);

    //             //wake on new work or shutdown
    //             queueCV.wait(lock, [this] {
    //                 return !ready_queue.empty() || !SchedulerRunning;
    //             });

    //             //no more work AND we shutting down --> exit
    //             if (!SchedulerRunning && ready_queue.empty()) break;

    //             //bobong fake awake or still no work? --> loop again
    //             if (ready_queue.empty()) continue;

    //             current = ready_queue.front();
    //             ready_queue.pop();
    //             // current.setCurrentCoreId(coreId); 
    //             runningProcesses.push_back(current);
    //         }

    //         current.setCurrentCoreId(coreId);
    //         current.setState(ProcessState::RUNNING);

    //         cout << "Running process ID: " << current.getPid() 
    //              << " on core ID: " << current.getCurrentCoreId() << "\n";
    //         //run the process instructions
    //         // current.runInstructions();

    //         //set state of process to FINISHED
    //         current.setState(ProcessState::FINISHED);
    //         cout << "Process ID: " << current.getPid() 
    //              << " has finished running on core ID: " << current.getCurrentCoreId() << "\n";

    //         {
    //             std::lock_guard<std::mutex> lock(queueMutex);

    //             //remove from running, push to completed
    //             auto it = std::find_if(runningProcesses.begin(), runningProcesses.end(),
    //                                 [&](const Process& p) { return p.getPid() == current.getPid(); });
    //             if (it != runningProcesses.end()) {
    //                 runningProcesses.erase(it);
    //             }
    //             completedProcesses.push_back(current);
    //         }
    //         checkIfComplete();
    //     }
    //     std::cout << "Worker thread for core " << coreId << " exiting.\n";
    // }
    //version 2


    // RR
    // void schedulerAlgo(int coreId) {
    //     while (generatingProcesses.load() || !ready_queue.empty()) {
    //         std::unique_lock<std::mutex> lock(queueMutex);
    //         queueCV.wait(lock, [this]{
    //         return !ready_queue.empty()
    //             || !generatingProcesses.load();
    //         });

    //         if (ready_queue.empty() && !generatingProcesses.load())
    //         break;

    //         // dequeue next process
    //         Process current = ready_queue.front();
    //         ready_queue.pop();
    //         lock.unlock();

    //         // ─── RUN YOUR ROUND-ROBIN TIME SLICE ───
    //         current.setState(ProcessState::RUNNING);
    //         unsigned slice = std::min<unsigned>(
    //             current.getRemainingBurst(),
    //             quantumCycles);
    //         // e.g. execute slice instructions (or cycles)
    //         for (unsigned i = 0; i < slice; ++i) {
    //         current.runInstructions();
    //         }
    //         current.setRemainingBurst(
    //         current.getRemainingBurst() - slice
    //         );

    //         if (current.getRemainingBurst() > 0) {
    //         // not done, re-enqueue
    //         current.setState(ProcessState::WAITING);
    //         std::lock_guard<std::mutex> reqlock(queueMutex);
    //         ready_queue.push(current);
    //         queueCV.notify_one();
    //         } else {
    //         // finished
    //         current.setState(ProcessState::FINISHED);
    //         std::lock_guard<std::mutex> complock(queueMutex);
    //         completedProcesses.push_back(current);
    //         }
    //     }
    //     std::cout << "Worker thread for core "
    //                 << coreId << " exiting.\n";
    // }


    // void Scheduler::startScheduler(int num_cpu) {
    void startScheduler(int num_cpu) {
        this->schedulerRunning = true;
         for (int coreId = 0; coreId < num_cpu; ++coreId) {
            workerThreads.emplace_back(&Scheduler::schedulerAlgo, this, coreId);
        }

        //version 1
        // // SchedulerRunning = true;
        // GeneratingProcesses = true;
        // while (SchedulerRunning) {
        //     //TODO: what coreID does the workerThread put the scheduler algorithm in????
        //     //-andrei
        //     //for now imma put it as the index of for loop
        //     std::cout << "to fix: startScheduler(), fix what coreID to put scheduler algorithm intto" << std::endl;
        //     workerThreads.emplace_back(&Scheduler::schedulerAlgo, this); //i variable here needs to be changed
        // }

        //version 2
        // SchedulerRunning.store(true);

        // for (int coreId = 0; coreId < num_cpu; ++coreId) {
        //     workerThreads.emplace_back([this, coreId]() {
        //         [this, coreId]() { this->schedulerAlgo(coreId); }
        //         std::cout << "Worker thread for core " << coreId << " started.\n";

        //         // // bind this thread to logical coreId
        //         // DWORD_PTR mask = (1ULL << coreId);
        //         // SetThreadAffinityMask(GetCurrentThread(), mask);

        //         // // now enter your scheduling loop
        //         // this->schedulerAlgo();
        //     });
        // }

        //version 3
        // schedulerRunning = true;
        // generatingProcesses = true;
        
        // for (int coreId = 0; coreId < num_cpu; ++coreId) {
            

        //     workerThreads.emplace_back([this,coreId](){
                
        //     });
        // }
    }

    void stopGenerating() {
        generatingProcesses = false;
        queueCV.notify_all();   // wake up any workers waiting
    }


    void stopScheduler() {
        // GeneratingProcesses = false;
        // std::cout << "Scheduler stop signal received. Waiting for all processes to finish...\n";

        // {
        //     std::lock_guard<std::mutex> lock(queueMutex);
        //     SchedulerRunning.store(false);
        // }
        // queueCV.notify_all();

        // for (auto &t : workerThreads)
        //     if (t.joinable())
        //         t.join();
        // workerThreads.clear();

        schedulerRunning = false;
        generatingProcesses = false;
        queueCV.notify_all();
        for (auto &t : workerThreads)
            if (t.joinable()) t.join();
        workerThreads.clear();


    }

    void finalizeScheduler() {
        schedulerRunning = false;
            for (auto& t : workerThreads) {
                if (t.joinable())
                    t.join();
            }
            std::cout << "Scheduler fully shut down. All processes completed.\n";
    }


    bool isSchedulerRunning() const {
        return schedulerRunning;
    }

    bool isGeneratingProcesses() const {
        return generatingProcesses;
    }

    // bool runningThreadsDone() {
    //     std::lock_guard<std::mutex> lock(queueMutex);
    //     return std::all_of(runningProcesses.begin(), runningProcesses.end(), [](const Process& p) {
    //         return p.getState() == ProcessState::FINISHED;
    //     });
    // }

    // void displayProcessList() {
    //     for (const auto& procPtr : processes) {
    //         const Process& proc = *procPtr;
    //         std::cout << "Process ID: " << proc.getPid() << "\n"
    //             << "Process Name: " << proc.getProcessName() << "\n"
    //             << "Current Core ID: " << proc.getCurrentCoreId() << "\n"
    //             << "Arrival Time: " << proc.getArrivalTime() << "\n"
    //             << "Burst Time: " << proc.getBurstTime() << "\n"
    //             << "Remaining Burst: " << proc.getRemainingBurst() << "\n"
    //             << "Waiting Time: " << proc.getWaitingTime() << "\n"
    //             << "Run Count: " << proc.getRunCount() << "\n"
    //             << "State: " << processStateToString(proc.getState()) << "\n"
    //             << "\n"
    //             << std::endl;
            
    //         proc.displayInstructionList();
    //     }
    // }

    // Runs FCFS on the given list of processes.
    //
    // For each process, this will:
    // 1) record its start/end times via appendStartTime/appendEndTime
    // 2) set remaining_burst to 0
    // 3) set waiting_time
    // and finally return the overall average waiting time.
    // struct CompareArrival {
    //     bool operator()(Process* a, Process* b) const {
    //         auto at_a = a->getArrivalTime();
    //         auto at_b = b->getArrivalTime();
    //         if (at_a == at_b)
    //             return a->getPid() > b->getPid();   // later PID => lower priority
    //         return at_a > at_b;                     // larger arrival_time => lower priority
    //     }
    // };

    // double FCFS() {
    //     if (processes.empty()) 
    //         return 0.0;

    //     // Build a min‐heap of Process* by arrival time
    //     std::priority_queue<
    //         Process*,
    //         std::vector<Process*>,
    //         CompareArrival
    //     > arrivalQ;

    //     for (auto& proc : processes) {
    //         arrivalQ.push(proc.get());
    //     }

    //     // Pop the very first process
    //     Process* prev = arrivalQ.top();
    //     arrivalQ.pop();

    //     // First process: starts at its arrival, no waiting
    //     prev->setStartTime(prev->getArrivalTime());
    //     prev->setEndTime(prev->getStartTime(0) + prev->getBurstTime());
    //     prev->setRemainingBurst(0);
    //     prev->setWaitingTime    (0);

    //     double aveWait = 0.0;
    //     size_t count  = 1;  // we’ll use this as the “i” in computeStreamAve

    //     // Now handle the rest in arrival order
    //     while (!arrivalQ.empty()) {
    //         Process* curr = arrivalQ.top();
    //         arrivalQ.pop();

    //         uint64_t start = prev->getEndTime(0);
    //         curr->setStartTime(start);

    //         uint64_t finish = start + curr->getBurstTime();
    //         curr->setEndTime(finish);

    //         curr->setRemainingBurst(0);

    //         uint64_t wait = finish
    //                     - curr->getArrivalTime()
    //                     - curr->getBurstTime();
    //         curr->setWaitingTime(wait);

    //         // update running average
    //         aveWait = updateRunningAverage(aveWait, wait, count);
    //         ++count;

    //         prev = curr;
    //     }

    //     return aveWait;
    // }
};