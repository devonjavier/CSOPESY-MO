#include <iostream>
#include <string>
#include <cstdlib> // for system()
#include <ctime>
#include "header.h"
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <iomanip>
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
    os_scheduler = new Scheduler(scheduler_type, quantumcycles, g_memory_manager, delays_perexec);
    

    config.close();
}

ICommand* generateRandomInstruction() {
    // Randomly choose an instruction type
    int instruction_type = rand() % 8; // 0 to 4 for 5 different types

    std::string randomVarName1 = "var" + std::to_string(rand() % 10);
    std::string randomVarName2 = "var" + std::to_string(rand() % 10);
    std::string resultVarName = "result" + std::to_string(rand() % 5);

    switch (instruction_type) {
        case 0: // PRINT
            if (rand() % 2 == 0) {
                return new PRINT(); 
            } else {
                return new PRINT(randomVarName1, true);
            }
        case 1: // DECLARE
            return new DECLARE(randomVarName1, rand() % 100);
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
        case 4: 
            return new SUBTRACT(resultVarName, randomVarName1, randomVarName2);
        case 5:
            return new ADD(resultVarName, randomVarName1, randomVarName2);
        case 6: { // READ
            // Generate a random memory address within the default process memory size
            uint32_t random_address = rand() % mem_per_proc; 
            return new READ(randomVarName1, random_address);
        }
        case 7: { // WRITE
            uint32_t random_address = rand() % mem_per_proc;
            // Write the value of a random, potentially undeclared variable
            return new WRITE(randomVarName1, random_address);
        }
        default:
            return new UNKNOWN;
    }
}
std::string parseStringLiteral(const std::string& input) {
    std::string trimmed = input;
    // Trim leading/trailing whitespace first
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);

    if (trimmed.length() < 2 || trimmed.front() != '"' || trimmed.back() != '"') {
        return ""; // Not a valid quoted string
    }

    // Return the content inside the outer quotes
    return trimmed.substr(1, trimmed.length() - 2);
}

std::vector<std::unique_ptr<ICommand>> parseInstructionString(const std::string& raw_instructions) {
    std::vector<std::unique_ptr<ICommand>> program;
    std::stringstream ss(raw_instructions);
    std::string instruction_token;

    while (std::getline(ss, instruction_token, ';')) {

        instruction_token.erase(0, instruction_token.find_first_not_of(" \t\n\r"));
        instruction_token.erase(instruction_token.find_last_not_of(" \t\n\r") + 1);

        if (instruction_token.empty()) continue;

        std::stringstream token_stream(instruction_token);
        std::string opcode;
        token_stream >> opcode;

        if (opcode == "DECLARE") {
            std::string varName;
            uint16_t value;
            if (token_stream >> varName >> value) {
                program.push_back(std::make_unique<DECLARE>(varName, value));
            } else return {}; 
        } 
        else if (opcode == "ADD") {
            std::string res, op1, op2;
            if (token_stream >> res >> op1 >> op2) {
                program.push_back(std::make_unique<ADD>(res, op1, op2));
            } else return {};
        }
        else if (opcode == "SUBTRACT") {
            std::string res, op1, op2;
            if (token_stream >> res >> op1 >> op2) {
                program.push_back(std::make_unique<SUBTRACT>(res, op1, op2));
            } else return {};
        }
        else if (opcode == "READ") {
            std::string varName;
            uint32_t address;
            if (token_stream >> varName >> std::hex >> address) {
                program.push_back(std::make_unique<READ>(varName, address));
            } else return {};
        }
        else if (opcode == "WRITE") {
             std::string varName;
            uint32_t address;

            if (token_stream >> std::hex >> address >> varName) {
                program.push_back(std::make_unique<WRITE>(varName, address));
            } else return {};
        }

        else if (opcode == "PRINT") {
        std::string content;
        std::getline(token_stream, content); // Get the rest of the line: (...)

        size_t open_paren = content.find('(');
        size_t close_paren = content.rfind(')');
        if (open_paren == std::string::npos || close_paren == std::string::npos) return {};

        content = content.substr(open_paren + 1, close_paren - open_paren - 1);

        size_t plus_pos = content.find('+');
        if (plus_pos != std::string::npos) {
            // Case: "literal" + variable
            std::string literal_part = content.substr(0, plus_pos);
            std::string varName_part = content.substr(plus_pos + 1);

            // Use our new helper to correctly parse the literal
            std::string literal = parseStringLiteral(literal_part);

            // Trim the variable part
            varName_part.erase(0, varName_part.find_first_not_of(" \t"));
            varName_part.erase(varName_part.find_last_not_of(" \t") + 1);

            if (literal.empty() || varName_part.empty()) {
                return {}; // Parsing failed
            }
            
            program.push_back(std::make_unique<PRINT>(literal, varName_part));

        } else {
            // Case: A single item (either a literal or a variable)
            std::string literal = parseStringLiteral(content);
            if (!literal.empty() || (content.find('"') != std::string::npos)) {
                // It was a valid quoted string
                program.push_back(std::make_unique<PRINT>(literal, true));
            } else {
                // It's a single variable
                content.erase(0, content.find_first_not_of(" \t"));
                content.erase(content.find_last_not_of(" \t") + 1);
                program.push_back(std::make_unique<PRINT>(content));
            }
        }
    } else if (opcode == "SLEEP") {
            int duration;
            if (token_stream >> duration) {
                program.push_back(std::make_unique<SLEEP>(duration));
            } else return {};
        }
        else if (opcode == "FOR") {
            int repeatCount;
            std::vector<std::unique_ptr<ICommand>> body;

            token_stream >> repeatCount;
            if (token_stream.peek() != '{') return {}; 

            char openBrace, closeBrace;
            token_stream >> openBrace; // Read the '{'
            if (openBrace != '{') return {}; 

            std::string body_instructions;
            std::getline(token_stream, body_instructions, '}'); // Read until '}'

            auto body_program = parseInstructionString(body_instructions);
            if (body_program.empty()) return {}; 

            for (auto& instr : body_program) {
                body.push_back(std::move(instr));
            }

            program.push_back(std::make_unique<FOR>(std::move(body), repeatCount));
        }
        else if (opcode == "UNKNOWN") {
            program.push_back(std::make_unique<UNKNOWN>());
        }
        else {
            return {}; 
        }
    }

    if (program.size() < 1 || program.size() > 50) {
        std::cout << "[Parser] Error: Instruction count must be between 1 and 50.\n";
        return {}; 
    }

    return program;
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

// overload, instruction parse
Process* create_new_process(std::string name, size_t mem_size, std::vector<std::unique_ptr<ICommand>> program) {
    if (!os_scheduler) return nullptr;

    auto proc = std::make_unique<Process>(g_next_pid, name, mem_size, mem_per_frame);
    Process* raw_ptr = proc.get();


    for (auto& instruction : program) {
        raw_ptr->addInstruction(std::move(instruction));
    }

    raw_ptr->setBurstTime();
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
    g_is_generating = false;
    std::cout << "Scheduler stopped.\n";
    system("pause");
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
        std::cout << "Max Instructions: " << (max_ins) << "\n";
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
    } else if (choice.rfind("screen -s", 0) == 0) {
        std::string name = choice.substr(10);

        Process* proc = create_new_process(name); 
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

                Process* proc_to_resume = os_scheduler->findProcessByName(name);

                if (proc_to_resume) {

                    proc_to_resume->runScreenInterface();
                    
   
                    clear_screen();
                    screen_init();
                    return; 
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

            std::vector<Process*> all_procs = os_scheduler->getAllProcesses();
            size_t active_ticks = os_scheduler->getActiveTicks();
            size_t total_ticks = os_scheduler->getTotalTicks();
            float cpu_util = (total_ticks > 0) ? (static_cast<float>(active_ticks) / total_ticks) * 100.0f : 0.0f;

            std::cout << " CPU Utilization : " << std::fixed << std::setprecision(2) << cpu_util << "%\n";

            std::cout << "\n--- Process List ---\n";
            // Ask the scheduler for a list of all processes
            
            if (all_procs.empty()) {
                std::cout << "  (No processes in system)\n";
            } else {
                // Use a stable width for formatting
                const int nameWidth = 20;
                const int pidWidth = 8;
                
                std::cout << std::left << std::setw(nameWidth) << "NAME" 
                        << std::setw(pidWidth) << "PID" << "STATUS\n";
                std::cout << "------------------------------------------\n";

                for (Process* proc : all_procs) {
                    if (proc) { // Safety first
                        std::cout << std::left << std::setw(nameWidth) << proc->getProcessName()
                                << std::setw(pidWidth) << proc->getPid()
                                << processStateToString(proc->getState()) << std::endl;
                    }
                }
            }
        }
        system("pause");

    } else if (choice.rfind("screen -c", 0) == 0) {
 
        size_t first_quote = choice.find('"');
        if (first_quote == std::string::npos) {
            std::cout << "Error: Invalid format. Instruction string must be enclosed in double quotes.\n";
            system("pause");
            return;
        }

        std::string command_part = choice.substr(0, first_quote);
        std::stringstream ss(command_part);
        std::string command, flag, name;
        size_t mem_size;
   
        ss >> command >> flag >> name >> mem_size;

        if (name.empty() || ss.fail()) {
            std::cout << "Error: Invalid format. Usage: screen -c <name> <size> \"<instructions>\"\n";
            system("pause");
            return;
        }
        
        size_t last_quote = choice.rfind('"');
        if (last_quote == first_quote) { // Only one quote found
            std::cout << "Error: Invalid format. Unmatched double quote in instruction string.\n";
            system("pause");
            return;
        }
        std::string raw_instructions = choice.substr(first_quote + 1, last_quote - first_quote - 1);

        bool is_power_of_two = (mem_size > 0) && ((mem_size & (mem_size - 1)) == 0);
        if (mem_size < 64 || mem_size > 65536 || !is_power_of_two) {
            std::cout << "Error: Invalid memory allocation. Size must be a power of 2 between 64 and 65536.\n";
            system("pause");
            return;
        }
        

        std::vector<std::unique_ptr<ICommand>> program = parseInstructionString(raw_instructions);

        if (program.empty()) {
            std::cout << "Error: Failed to parse instruction string or instruction count is invalid.\n";
        } else {
            if (os_scheduler && os_scheduler->findProcessByName(name)) {
                std::cout << "Error: Process with that name already exists.\n";
            } else {
                create_new_process(name, mem_size, std::move(program));
                std::cout << "Process '" << name << "' created successfully with custom instructions.\n";
            }
        }
        system("pause");
    } else if (choice == "exit") {
        *current = OSState::EXITING;
        std::cout << "Exiting the OS...\n";
        system("pause");
    } else if (choice == "clear") { 
        clear_screen();
        screen_init();
    } else if (choice == "process-smi") {
        if (!g_memory_manager || !os_scheduler) {
            std::cout << "Error: System not fully initialized. Please run 'initialize' first.\n";
        } else {
            size_t total_mem = g_memory_manager->getTotalMemory();
            size_t used_mem = g_memory_manager->getUsedMemory();
            
            float mem_util = (total_mem > 0) ? (static_cast<float>(used_mem) / total_mem) * 100.0f : 0.0f;
            
            size_t active_ticks = os_scheduler->getActiveTicks();
            size_t total_ticks = os_scheduler->getTotalTicks();
            float cpu_util = (total_ticks > 0) ? (static_cast<float>(active_ticks) / total_ticks) * 100.0f : 0.0f;

            clear_screen();
            std::cout << "+-----------------------------------------------------------------------------+\n";
            std::cout << "| PROCESS-SMI V1.0                Driver Version: 1.0                           |\n";
            std::cout << "|-------------------------------+----------------------+----------------------+\n";
            std::cout << "| CPU Util.                     | Memory Usage         |                      |\n";
            std::cout << "|===============================+======================+======================|\n";
            

            std::cout << "| " << std::fixed << std::setprecision(2) << std::setw(7) << cpu_util << "%   Off  |  ";
            std::cout << std::setw(7) << (used_mem / 1024) << "KiB / " << std::setw(7) << (total_mem / 1024) << "KiB | ";
            std::cout << std::setw(7) << std::fixed << std::setprecision(2) << mem_util << "% Usage       |\n";
            
            std::cout << "+-----------------------------------------------------------------------------+\n";
            
            std::vector<Process*> all_procs = os_scheduler->getAllProcesses();

            std::cout << "| Processes:                                                                  |\n";
            std::cout << "|  PID       Name                 State                Memory Usage           |\n";
            std::cout << "|=============================================================================|\n";

            if (all_procs.empty()) {
                std::cout << "|  No running processes.                                                      |\n";
            } else {
                for (Process* proc : all_procs) {
                    if (proc) {

                        std::cout << "|  " << std::left << std::setw(10) << proc->getPid()
                                  << std::setw(21) << proc->getProcessName()
                                  << std::setw(21) << processStateToString(proc->getState());
                        std::stringstream mem_ss;
                        mem_ss << proc->getMemorySize() << " Bytes";
                        std::cout << std::setw(22) << mem_ss.str() << "|\n";
                    }
                }
            }
            std::cout << "+-----------------------------------------------------------------------------+\n";
        }
        system("pause");  
    } else if (choice == "report-util") {

    } else if (choice == "vmstat") {
        if (!g_memory_manager || !os_scheduler) {
            std::cout << "Error: System not fully initialized. Please run 'initialize' first.\n";
        } else {

            size_t total_mem = g_memory_manager->getTotalMemory();
            size_t used_mem = g_memory_manager->getUsedMemory();
            size_t free_mem = g_memory_manager->getFreeMemory();

            size_t paged_in = g_memory_manager->getNumPagedIn();
            size_t paged_out = g_memory_manager->getNumPagedOut();

            size_t active_ticks = os_scheduler->getActiveTicks();
            size_t idle_ticks = os_scheduler->getIdleTicks();
            size_t total_ticks = os_scheduler->getTotalTicks();


            const int label_width = 18; 
        
            std::cout << std::left << std::setw(label_width) << "Total Memory:" << total_mem << " bytes\n";
            std::cout << std::left << std::setw(label_width) << "Used Memory:"  << used_mem << " bytes\n";
            std::cout << std::left << std::setw(label_width) << "Free Memory:"  << free_mem << " bytes\n";
            
            std::cout << std::left << std::setw(label_width) << "Pages Paged In:" << paged_in << "\n";
            std::cout << std::left << std::setw(label_width) << "Pages Paged Out:" << paged_out << "\n";
            

            std::cout << std::left << std::setw(label_width) << "Active Ticks:" << active_ticks << "\n";
            std::cout << std::left << std::setw(label_width) << "Idle Ticks:" << idle_ticks << "\n";
            std::cout << std::left << std::setw(label_width) << "Total Ticks:" << total_ticks << "\n";

        }
        system("pause");
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
        *current = OSState::MAIN_MENU;
        *active_session = nullptr;    
        clear_screen();
        screen_init(); 
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
        g_is_generating = false;
        if (g_process_generator_thread.joinable()) {
            g_process_generator_thread.join(); 
        }
        delete os_scheduler;

    }
}
