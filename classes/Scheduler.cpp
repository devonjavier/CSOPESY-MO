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

    void startScheduler(int num_cpu) {
        this->schedulerRunning = true;
         for (int coreId = 0; coreId < num_cpu; ++coreId) {
            workerThreads.emplace_back(&Scheduler::schedulerAlgo, this, coreId);
        }

    }

    void stopGenerating() {
        generatingProcesses = false;
        queueCV.notify_all();   
    }


    void stopScheduler() {


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
};