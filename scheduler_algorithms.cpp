#include "scheduler.h"
#include <sstream>
#include <iomanip>


Scheduler::Scheduler(int count, const std::string& schedulerType, int timeQuantum, int delayPerInstruction) 
    : coreCount(count), 
      schedulerType(schedulerType), 
      timeQuantum(timeQuantum), 
      delayPerInstruction(delayPerInstruction),
      cores(count, nullptr) 
{}

//override scheduler constructor to handle case when no timeQuantum is given if type is fcfs
Scheduler::Scheduler(int count, const std::string& schedulerType, int delayPerInstruction) 
    : coreCount(count), 
      schedulerType(schedulerType), 
      delayPerInstruction(delayPerInstruction),
      cores(count, nullptr) 
{}


void Scheduler::addProcess(Process* p) 
{
    std::unique_lock<std::mutex> lock(mtx); 
    processQueue.push(p);
    cv.notify_all();
}

// Starts the scheduler by launching a thread for each CPU core.
void Scheduler::start() 
{
    for (int i = 0; i < coreCount; ++i) 
    {
        coreThreads.emplace_back(&Scheduler::coreWorker, this, i);
    }
}

// The main function executed by each CPU core thread.
// Implements the First-Come, First-Served (FCFS) scheduling logic.
void Scheduler::coreWorker(int coreId)
{
    while (true)
    {
        Process* currentProcess = nullptr;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !processQueue.empty() || shutdown; });

            if (shutdown && processQueue.empty())
            {
                cores[coreId] = nullptr;
                break;
            }

            if (!processQueue.empty())
            {
                currentProcess = processQueue.front();
                processQueue.pop();
                cores[coreId] = currentProcess;
            }
        }

        if (currentProcess)
        {
            // Mark as running
            currentProcess->setState(Process::RUNNING);

            if (schedulerType == "fcfs")
            {
                while (!currentProcess->isFinished())
                {
                    if (shutdown) break;
                    currentProcess->executeCurrentCommand(coreId);
                    currentProcess->moveToNextLine();
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayPerInstruction));
                }
            }
            else if (schedulerType == "rr")
            {
                for (int i = 0; i < timeQuantum && !currentProcess->isFinished(); ++i)
                {
                    if (shutdown) break;
                    currentProcess->executeCurrentCommand(coreId);
                    currentProcess->moveToNextLine();
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayPerInstruction));
                }
            }

            std::unique_lock<std::mutex> lock(mtx);
            if (currentProcess->isFinished())
            {
                currentProcess->setState(Process::FINISHED);
                currentProcess->setFinishTime(currentTimestamp());
                finishedProcesses.push_back(currentProcess);
            }
            else
            {
                currentProcess->setState(Process::READY);
                processQueue.push(currentProcess);  // RR puts it back if not finished
            }

            cores[coreId] = nullptr;
        }
    }
}

void Scheduler::stop() 
{
    {
        std::unique_lock<std::mutex> lock(mtx); // Acquire lock
        shutdown = true; // Set shutdown flag
    } // Lock is released here
    cv.notify_all(); // Notify all waiting core threads
}

// Waits for all core threads to finish their execution.
void Scheduler::wait() 
{
    for (auto& t : coreThreads) 
    {
        if (t.joinable()) t.join(); // Join each thread
    }
}

// Returns the current timestamp formatted as YYYY-MM-DD HH:MM:SS.
std::string Scheduler::currentTimestamp() 
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    // Use std::put_time for formatting the time
    ss << std::put_time(std::localtime(&in_time_t), "(%m/%d/%Y %H:%M:%S %p)");
    return ss.str();
}

// Returns a vector of strings, each representing the status of a CPU core.
std::vector<std::string> Scheduler::getCoreStatus()
{
    std::vector<std::string> status;
    std::unique_lock<std::mutex> lock(mtx); // Acquire lock to access cores vector

    for (int i = 0; i < coreCount; ++i)
    {
        if (cores[i] != nullptr)
        {
            status.push_back("Core " + std::to_string(i) + ": Running " + cores[i]->getName());
        }
        else
        {
            status.push_back("Core " + std::to_string(i) + ": Idle");
        }
    }
    return status;
}

std::vector<std::string> Scheduler::getRunningProcesses()
{
    std::vector<std::string> status;
    std::unique_lock<std::mutex> lock(mtx); // Lock to safely access cores

    for (int i = 0; i < cores.size(); ++i)
    {
        if (cores[i])  // if core is working on a process
        {
            Process* p = cores[i];
            int done = p->getCommandCounter();
            int total = p->getLinesOfCode();
            std::string time_now = currentTimestamp();

            status.push_back(p->getName() + "   " + time_now + "   Core: " + std::to_string(i) + "   " +
                             std::to_string(done) + "/" + std::to_string(total));
        }
    }
    return status;
}


// Returns a vector of strings, each representing a finished process.
std::vector<std::string> Scheduler::getFinishedProcesses()
{
    std::vector<std::string> status;
    std::unique_lock<std::mutex> lock(mtx); // Acquire lock

    for (const auto& p : finishedProcesses)
    {
        int total = p->getLinesOfCode();
        status.push_back(p->getName() + "   " + p->getFinishTime() + "   Finished   " +
                         std::to_string(total) + "/" + std::to_string(total));
    }

    return status;
}

int Scheduler::getUsedCores() {
    std::unique_lock<std::mutex> lock(mtx);
    int used = 0;
    for (const auto& p : cores) {
        if (p != nullptr) used++;
    }
    return used;
}

