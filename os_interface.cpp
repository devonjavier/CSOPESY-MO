#include <iostream>
#include <string>
#include <cstdlib> // for system()
#include <ctime>
#include "header.h"
#include "classes/helper.h"
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include "classes/MemoryManager.cpp"
#include <fstream>
#include <sstream>
#include <random> // For random number generation
#include <chrono> // For random number seeding with time
#include <cmath> 
#include <iomanip>

using namespace std;

enum class OSState {
    MAIN_MENU,
    SCREEN_SESSION,
    EXITING
};


//initialization of variables
int num_cpu = 0;
std::string scheduler_type = "";
int quantumcycles = 0;
int batchprocess_freq = 0;
int min_ins = 0;
int max_ins = 0;
int delays_perexec = 0;

// new parameters for OS emulator memory manager
int max_overall_mem = 0;
int mem_per_frame = 0;
int mem_per_proc = 0;

//initialization of Screens and Processes Lists
Scheduler* os_scheduler = nullptr;

// Global memory manager pointer
MemoryManager* g_memory_manager = nullptr;

// p_id
int g_next_pid = 1;
// process generator thread
std::thread g_process_generator_thread;
// process generator thread flag
bool g_is_generating = false;


std::mutex screenListMutex;

static const std::vector<ProcessState> stateOrder = {
    ProcessState::IDLE,
    ProcessState::WAITING,
    ProcessState::RUNNING,
    ProcessState::FINISHED,
    ProcessState::TERMINATED
};

//initial declaration (maybe transfer to a header file)
// std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp) {
//     if (tp.time_since_epoch().count() == 0) return "N/A";
//     std::time_t tt = std::chrono::system_clock::to_time_t(tp);
//     char buffer[100];
//     std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %I:%M:%S%p", std::localtime(&tt));
//     return std::string(buffer);
// }


ICommand* generateRandomInstruction() {
    // Randomly choose an instruction type
    int instruction_type = rand() % 6; // 0 to 4 for 5 different types

    switch (instruction_type) {
        case 0: // PRINT
            return new PRINT(); 
        case 1: // DECLARE
            return new DECLARE("var" + std::to_string(rand() % 100), rand() % 100);
        case 2: // SLEEP
            return new SLEEP(rand() % 50 + 1);
        case 3: {
            int loop_count = rand() % 3 + 1; 
            std::vector<std::unique_ptr<ICommand>> body;

            int body_instr_count = rand() % 3 + 1;
            for (int i = 0; i < body_instr_count; ++i) {
                body.push_back(std::unique_ptr<ICommand>(generateRandomInstruction()));
            }

            return new FOR(std::move(body), loop_count);
        }
        case 4: // SUBTRACT with random variables or values
            return new SUBTRACT("result", "var" + std::to_string(rand() % 100), "var" + std::to_string(rand() % 100));
        case 5:
            return new ADD("result", "var" + std::to_string(rand() % 100), "var" + std::to_string(rand() % 100));
        default:
            return new UNKNOWN;
    }
}

Process* create_new_process(std::string name) {
    std::default_random_engine generator(
        std::chrono::system_clock::now().time_since_epoch().count()
    );

    std::uniform_int_distribution<int> instructionDist(min_ins, max_ins);
    int num_instructions = instructionDist(generator);

    auto proc = std::make_unique<Process>(g_next_pid, name); 

    //TEMP 
    Process* raw_ptr = proc.get();
    
    for (int i = 0; i < num_instructions; ++i) {
        proc->addInstruction(std::unique_ptr<ICommand>(generateRandomInstruction())); 
    }

    raw_ptr->setBurstTime(); // calc burst time
    raw_ptr->setRemainingBurst(raw_ptr->getBurstTime());
    

    os_scheduler->addProcess(std::move(proc));
    g_next_pid++;
    
    return raw_ptr; 
}


void generate_random_processes() {
    for (int i = 0; i < batchprocess_freq; ++i) {
        Process* new_proc_ptr = create_new_process("Process" + std::to_string(g_next_pid)); 
    }
}

void initialize() {

    std::ifstream config("config.txt");
    if (!config.is_open()) {
        std::cerr << "Error: Could not open config.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(config, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue; 

        for (char &c : key) {
            c = std::tolower(static_cast<unsigned char>(c));
        }

        if (key == "num-cpu") {
            iss >> num_cpu;
            if (num_cpu < 1 || num_cpu > 128) {
                std::cerr << "Invalid num-cpu value. Must be in [1,128]." << std::endl;
            }
        } else if (key == "scheduler") {
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            rest_iss >> scheduler_type;

            if (scheduler_type != "fcfs" && scheduler_type != "rr") {
                std::cerr << "Invalid Scheduler value. Must be 'fcfs' or 'rr'." << std::endl;
            }

            // Optional: print what's after "Scheduler"
            std::string remaining_args;
            std::getline(rest_iss, remaining_args);
            if (!remaining_args.empty()) {
                std::cout << "Extra arguments after Scheduler: " << remaining_args << std::endl;
            }

        } else if (key == "quantumcycles") {
            iss >> quantumcycles;
            if (quantumcycles < 1) {
                std::cerr << "Invalid quantumcycles value. Must be >=1." << std::endl;
            }
        } else if (key == "batchprocess-freq") {
            iss >> batchprocess_freq;
            if (batchprocess_freq < 1) {
                std::cerr << "Invalid batchprocess-freq value. Must be >=1." << std::endl;
            }
        } else if (key == "min-ins") {
            iss >> min_ins;
            if (min_ins < 1) {
                std::cerr << "Invalid min-ins value. Must be >=1." << std::endl;
            }
        } else if (key == "max-ins") {
            iss >> max_ins;
            if (max_ins < 1) {
                std::cerr << "Invalid max-ins value. Must be >=1." << std::endl;
            }
        } else if (key == "delays-perexec") {
            iss >> delays_perexec;
        } else if (key == "max-overall-mem") {
            iss >> max_overall_mem;
            if (max_overall_mem < 1) {
                std::cerr << "Invalid max-overall-mem value. Must be >=1." << std::endl;
            }
        } else if (key == "mem-per-frame") {
            iss >> mem_per_frame;
            if (mem_per_frame < 1) {
                std::cerr << "Invalid mem-per-frame value. Must be >=1." << std::endl;
            }
        } else if (key == "mem-per-proc") {
            iss >> mem_per_proc;
            if (mem_per_proc < 1) {
                std::cerr << "Invalid mem-per-proc value. Must be >=1." << std::endl;
            }
        } else {
            std::cerr << "Unknown configuration key: " << key << std::endl;

        }
    };
    g_memory_manager = new MemoryManager(max_overall_mem, mem_per_frame, mem_per_proc);
    os_scheduler = new Scheduler(scheduler_type, quantumcycles, g_memory_manager);
    

    config.close();
}



ICommand* generateRandomInstruction() {
    // Randomly choose an instruction type
    int instruction_type = rand() % 8; // 0 to 4 for 5 different types

    switch (instruction_type) {
        case 0: // PRINT
            return new PRINT(); 
        case 1: // DECLARE
            return new DECLARE("var" + std::to_string(rand() % 100), rand() % 100);
        case 2: // SLEEP
            return new SLEEP(rand() % 50 + 1);
        case 3: {
            int loop_count = rand() % 3 + 1; 
            std::vector<std::unique_ptr<ICommand>> body;

            int body_instr_count = rand() % 3 + 1;
            for (int i = 0; i < body_instr_count; ++i) {
                body.push_back(std::unique_ptr<ICommand>(generateRandomInstruction()));
            }

            return new FOR(std::move(body), loop_count);
        }
        case 4: // SUBTRACT with random variables or values
            return new SUBTRACT("result", "var" + std::to_string(rand() % 100), "var" + std::to_string(rand() % 100));
        case 5:
            return new ADD("result", "var" + std::to_string(rand() % 100), "var" + std::to_string(rand() % 100));
        case 6: { // READ
            // Generate a random memory address within the default process memory size
            uint32_t random_address = rand() % mem_per_proc; 
            return new READ("var_read", random_address);
        }
        case 7: { // WRITE
            uint32_t random_address = rand() % mem_per_proc;
            // Write the value of a random, potentially undeclared variable
            std::string random_var = "var" + std::to_string(rand() % 100);
            return new WRITE(random_var, random_address);
        }
        default:
            return new UNKNOWN;
    }
}

Process* create_new_process(std::string name) {
    std::default_random_engine generator(
        std::chrono::system_clock::now().time_since_epoch().count()
    );

    std::uniform_int_distribution<int> instructionDist(min_ins, max_ins);
    int num_instructions = instructionDist(generator);

    auto proc = std::make_unique<Process>(g_next_pid, name, mem_per_proc, mem_per_frame);

    //TEMP 
    Process* raw_ptr = proc.get();
    
    for (int i = 0; i < num_instructions; ++i) {
        proc->addInstruction(std::unique_ptr<ICommand>(generateRandomInstruction())); 
    }

    raw_ptr->setBurstTime(); // calc burst time
    raw_ptr->setRemainingBurst(raw_ptr->getBurstTime());
    

    os_scheduler->addProcess(std::move(proc));
    g_next_pid++;
    
    return raw_ptr; 
}


void generate_random_processes() {
    for (int i = 0; i < batchprocess_freq; ++i) {
        Process* new_proc_ptr = create_new_process("Process" + std::to_string(g_next_pid)); 
    }
}

// 2. screen_init()
void screen_init() {


    // credits to https://patorjk.com/software/taag/#p=display&f=Slant%20Relief&t=CSOPESY for the unmodified ASCII art
    std::cout << "________/\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\_________/\\\\\\\\\\_______/\\\\\\\\\\\\\\\\\\\\\\\\\\____/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\____/\\\\\\________/\\\\\\_\n";
    std::cout << " _____/\\\\\\////////____/\\\\\\/////////\\\\\\_____/\\\\\\///\\\\\\____\\/\\\\\\/////////\\\\\\_\\/\\\\\\///////////____/\\\\\\/////////\\\\\\_\\///\\\\\\____/\\\\\\/__\n";
    std::cout << "  ___/\\\\\\/____________\\//\\\\\\______\\///____/\\\\\\/__\\///\\\\\\__\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\______________\\//\\\\\\______\\///____\\///\\\\\\/\\\\\\/____\n";
    std::cout << "   __/\\\\\\_______________\\////\\\\\\__________/\\\\\\______\\//\\\\\\_\\/\\\\\\\\\\\\\\\\\\\\\\\\\\/__\\/\\\\\\\\\\\\\\\\\\\\\\_______\\////\\\\\\_____________\\///\\\\\\/______\n";
    std::cout << "    _\\/\\\\\\__________________\\////\\\\\\______\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\/////////____\\/\\\\\\///////___________\\////\\\\\\____________\\/\\\\\\_______\n";
    std::cout << "     _\\//\\\\\\____________________\\////\\\\\\___\\//\\\\\\______/\\\\\\__\\/\\\\\\_____________\\/\\\\\\_____________________\\////\\\\\\_________\\/\\\\\\_______\n";
    std::cout << "      __\\///\\\\\\___________/\\\\\\______\\//\\\\\\___\\///\\\\\\__/\\\\\\____\\/\\\\\\_____________\\/\\\\\\______________/\\\\\\______\\//\\\\\\________\\/\\\\\\_______\n";
    std::cout << "       ____\\////\\\\\\\\\\\\\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/______\\///\\\\\\\\\\/_____\\/\\\\\\_____________\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/_________\\/\\\\\\_______\n";
    std::cout << "        _______\\/////////____\\///////////__________\\/////_______\\///______________\\///////////////____\\///////////___________\\///________\n";
    std::cout << "\n\nHello! Welcome to the CSOPESY destroyers' command-line operating system!\n";
    std::cout << "";
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen, or 'help' for a list of commands.\n";
    std::cout << "Please enter a command:\n";
}


void scheduler_start() {
    if (!os_scheduler) {
        std::cout << "Scheduler not initialized.\n";
        return;
    }

    // 2 flags?  communication, dunno how to optimize
    os_scheduler->startScheduler(num_cpu);
    g_is_generating = true; 

    g_process_generator_thread = std::thread([]() {
        while (g_is_generating) { // Loop is controlled by our flag
            for (int i = 0; i < batchprocess_freq; ++i) {
                create_new_process("Process" + std::to_string(g_next_pid));
            }
            os_scheduler->queueProcesses(); 
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
}


void scheduler_stop() {
    // Scheduler_running = false;
    os_scheduler->stopGenerating();
    std::cout << "Scheduler stopped.\n";
}


void report_util() {
    std::ofstream log("csopesy-log.txt", std::ios::app);
    log << "===== Report (" << get_timestamp() << ") =====\n";

    //write the same stats as screen-ls
    std::cout << "Memory Usage: __ / __ KB\n";
    std::cout << "CPU Usage: __%\n";

    log.close();
    std::cout << "System report saved to csopesy-log.txt\n";
}


void accept_main_menu_input(std::string choice, OSState* current, Process** active_process) {
    if (choice == "initialize") {
        // initialize os env

        initialize();
        std::cout << std::endl << std::endl;
        std::cout << "Initialized configuration: \nCPU Cores: " << num_cpu << "\n";
        std::cout << "Scheduler: " << scheduler_type << "\n";
        std::cout << "Quantum Cycles: " << quantumcycles << "\n";
        std::cout << "Batch Process Frequency: " << batchprocess_freq << "\n";
        std::cout << "Min Instructions: " << min_ins << "\n";
        std::cout << "Max Instructions: " << max_ins << "\n";
        std::cout << "Delays per Execution: " << delays_perexec << "\n\n";
        std::cout << "Max Overall Memory: " << max_overall_mem << "\n";
        std::cout << "Memory per Frame: " << mem_per_frame << "\n";
        std::cout << "Memory per Process: " << mem_per_proc << "\n\n\n\n";
        system("pause");
    } else if (choice == "scheduler-start") {
        scheduler_start();
        system("pause");
    } else if (choice == "scheduler-stop") {
        scheduler_stop();
        system("pause");
    } else if (choice == "screen") {
        std::cout<<"Usage:\n"
            <<"  screen -s <name>  # attach/create session\n"
            <<"  screen -ls        # list sessions\n"
            <<"  screen -r <name>  # re-attach session\n";
        system("pause");
    } else if (choice.rfind("screen -s", 0) == 0) {
        std::string name = choice.substr(10); // get process name

        Process* proc = create_new_process(name); // what to do with process pointer???
        proc->runScreenInterface();
        system("pause");
    } else if (choice.rfind("screen -r", 0) == 0){ 
        if (!os_scheduler) {
            std::cout << "Scheduler not initialized.\n";
            system("pause");
        } else {
            std::string name = choice.substr(10);
            if (name.empty()) {
                std::cout << "Error: Process name not specified.\n";
                system("pause");
            } else {
                // Ask the scheduler to find the process
                Process* proc_to_resume = os_scheduler->findProcessByName(name);

                proc_to_resume->getState() != ProcessState::FINISHED;
                if (proc_to_resume) {
                    // We found it! Block and run its UI.
                    proc_to_resume->runScreenInterface();
                    
                    // After the user exits, clean up the main menu screen.
                    clear_screen();
                    screen_init();
                    return; // Return to avoid double-printing the banner
                } else {
                    std::cout << "Process '" << name << "' not found.\n";
                    system("pause");
                }
            }
        }
    } else if (choice.rfind("screen -ls", 0) == 0) {
        if (!os_scheduler) {
            std::cout << "Scheduler not initialized.\n";
        } else {
            float util      = os_scheduler->computeUtilization(num_cpu);
            int   usedCores = os_scheduler->numBusyCores();
            int   freeCores = num_cpu - usedCores;
            std::cout << "CPU Utilization: " << util << "%\n";
            std::cout << "Cores used: " << usedCores << ", available: " << freeCores << "\n\n";

            std::cout << "\n--- Process List ---\n";
            // Ask the scheduler for a list of all processes
            std::vector<Process*> all_procs = os_scheduler->getAllProcesses();

            if (all_procs.empty()) {
                std::cout << "  (No processes in system)\n";
            } else {
                // Use a stable width for formatting
                const int nameWidth = 20;
                const int pidWidth = 8;
                const int coreWidth = 6;    //idk i havent tested -Andrei


                for (ProcessState state : stateOrder) {
                    // Header for this state
                    std::cout << "--- " 
                            << processStateToString(state) 
                            << " Processes ---\n";
                    // Column titles
                    std::cout << std::left << std::setw(nameWidth) << "NAME" 
                            << std::setw(pidWidth) << "PID" << "STATUS"
                            << std::setw(coreWidth) << "CORE"
                            << "BURST (rem)\n";
                    // std::cout << "------------------------------------------\n";
                    std::cout << std::string(nameWidth+pidWidth+coreWidth+13, '-') << "\n";

                    // Print only those in this state
                    for (Process* proc : all_procs) {
                        if (!proc || proc->getState() != state) 
                            continue;
                        // if (proc) { // Safety first
                            std::cout << std::left << std::setw(nameWidth) << proc->getProcessName()
                                << std::setw(pidWidth) << proc->getPid();
                            if (state == ProcessState::RUNNING) {
                                std::cout << std::setw(coreWidth) 
                                        << proc->getCurrentCoreId();
                            } else {
                                std::cout << std::setw(coreWidth) << "-";
                            }
                            std::cout << proc->getBurstTime() << " / " << proc->getRemainingBurst() << "\n";
                    }
                    std::cout << "\n";
                }
            }
        }
        system("pause");
    } else if (choice == "report-util") {
        report_util();
        system("pause");
    } else if (choice == "exit") {
        *current = OSState::EXITING;
        std::cout << "Exiting the OS...\n";
        system("pause");
    } else if (choice == "clear") { 
        clear_screen();
        screen_init();
    }

    if (*current != OSState::SCREEN_SESSION) {
         clear_screen();
         screen_init();
    }

}


void accept_screen_session_input(std::string choice, OSState* current, Process** active_session){
        if (choice == "process-smi") {


        system("pause");
    } else if (choice == "exit") {
        *current = OSState::MAIN_MENU; // Go back to the main menu
        *active_session = nullptr;    // De-select the current session
        clear_screen();
        screen_init(); // Show the main menu banner again
        std::cout << "--- Returned to main menu ---\n";
    } else {
        std::cout << "Unknown screen command: " << choice << "\n";
    }


}


void menu(){
    clear_screen();
    OSState current = OSState::MAIN_MENU;
    std::string choice;

    Process* active_process = nullptr;

    while(current != OSState::EXITING){

        clear_screen();
        screen_init();

        if (!std::getline(std::cin, choice)) {
            current = OSState::EXITING;
            continue;
        }
        
        accept_main_menu_input(choice, &current, &active_process);
       
    }

    if(os_scheduler != nullptr) {
        os_scheduler -> stopGenerating();
        g_is_generating = false;
        if (g_process_generator_thread.joinable()) {
            g_process_generator_thread.join(); 
        }
        delete os_scheduler;

    }
}
