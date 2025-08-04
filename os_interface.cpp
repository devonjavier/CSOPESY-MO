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
#include "classes/Scheduler.cpp"
#include <fstream>
#include <sstream>
#include <random> // For random number generation
#include <chrono> // For random number seeding with time
#include <cmath> 

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
Process *head = nullptr;
Scheduler* os_scheduler = nullptr;

// p_id
int g_next_pid = 1;


//protect link list of session instance/screen list
std::mutex screenListMutex;

//initial declaration (maybe transfer to a header file)
void create_process_screen(const std::string& name, int total_lines);
void new_screen(std::string name);

// std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp) {
//     if (tp.time_since_epoch().count() == 0) return "N/A";
//     std::time_t tt = std::chrono::system_clock::to_time_t(tp);
//     char buffer[100];
//     std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %I:%M:%S%p", std::localtime(&tt));
//     return std::string(buffer);
// }

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", std::localtime(&now_time));
    return std::string(buffer);
}

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
    
    std::cout << "  -> Creating Process ID " << proc->getPid() 
              << " with " << num_instructions << " instructions.\n";
    
    for (int i = 0; i < num_instructions; ++i) {
        proc->addInstruction(std::unique_ptr<ICommand>(generateRandomInstruction())); 
    }
    
    std::lock_guard<std::mutex> lock(screenListMutex);
    if (head == nullptr) {
        head = proc.get(); 
    } else {
        Process* current = head;
        while (current->getNext() != nullptr) {
            current = current->getNext();
        }
        current->setNext(proc.get()); 
    }
    os_scheduler->addProcess(std::move(proc));
    g_next_pid++;
    
    return head; 
}


void generate_random_processes() {
    for (int i = 0; i < batchprocess_freq; ++i) {
        cout << (i + 1) << " out of " << batchprocess_freq << " processes being generated.\n";
        Process* new_proc_ptr = create_new_process("Process" + std::to_string(g_next_pid)); 
    }
}


void initialize() {
        // Gemini example:
        // Initialize GDT (or IDT) - architecture-specific!
        // gdt_init();
        // idt_init();

        // Set up a minimal stack (architecture-specific!)
        // stack_init();

        // Basic memory setup (e.g., identity mapping)
        // memory_init();

    std::ifstream config("config.txt");
    if (!config.is_open()) {
        std::cerr << "Error: Could not open config.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(config, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue; // Skip empty lines

        //convert choice to lowercase for case-insensitive comparison
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

    os_scheduler = new Scheduler(scheduler_type, quantumcycles);

    config.close();
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

// 3. scheduler_start()
//    - set up a test environment for the Scheduler
//    - create several test processes or threads
//    - according to the zoom kanina, each test process/thread will do some simple task
//
//    - starts the Scheduler
void scheduler_start() {
    os_scheduler->startScheduler(num_cpu);

    std::thread process_generator([]() {
        while (os_scheduler->isGeneratingProcesses()) {
            std::cout << "[Process Generator] Generating new batch of processes...\n";
            generate_random_processes();
            os_scheduler->queueProcesses();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    os_scheduler->checkIfComplete();

    process_generator.detach();  
}


// 4. scheduler_stop()
//    -stops process generation, lets worker threads finish their current tasks
void scheduler_stop() {
    // Scheduler_running = false;
    std::cout << "Scheduler stopped.\n";
}

// 5. report_util()
//    - report system information a d statistics
//        - Memory usage (total, used, free)
//        - CPU usage
//        - Running processes/threads
//        - System uptime
//        - Error logs

void report_util() {
    std::ofstream log("csopesy-log.txt", std::ios::app);
    log << "===== Report (" << get_timestamp() << ") =====\n";

    //write the same stats as screen-ls
    std::cout << "Memory Usage: __ / __ KB\n";
    std::cout << "CPU Usage: __%\n";

    log.close();
    std::cout << "System report saved to csopesy-log.txt\n";
}

// 6. clear_screen()
//    - clears the entire terminal screen
void clear_screen() {
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

// 7. exit_os(int status)
//    - shut down the operating system
//        - stop Scheduler
//        - unmount file systems (??idk)
//        - disable hardware devices (??idk)
//        - free memory
//        - close open files
//        - save system state
//        - log shutdown information
//        - notify other processes
//        - clean up resources
//        - stop all running processes
//        - disable interrupts
//        - set CPU to a known state
//        - return control to bootloader or firmware
//        - THEN halt CPU
//    - int status parameter to indicate exit status (egg. 0 for success, non-zero for an error).
void exit_os(int status) {
    // umount_all();     // Unmount file systems
    // disable_devices();
    // free_memory();   // Free allocated memory
    // close_open_files(); // Close open files
    // save_system_state(); // Save system state
    // log_shutdown_info(); // Log shutdown information
    // notify_processes(); // Notify other processes
    // clean_up_resources(); // Clean up resources
    // stop_all_processes(); // Stop all running processes
    // disable_interrupts(); // Disable interrupts
    // set_cpu_state(); // Set CPU to a known state
    // return_control_to_bootloader(); // Return control to bootloader or 
    scheduler_stop();
    std::exit(status);
}




// void generate_file(int core_id){
//     int current_file = file_count.fetch_add(1);
//     if (current_file >= num_processes) return;

//     std::string filename = "process_file_" + std::to_string(current_file) + "_id_" + std::to_string(core_id) + ".txt";
//     std::string start_time = get_timestamp();

//     {
//         std::lock_guard<std::mutex> lock(process_mutex);
//         processes.push_back({current_file, filename, "Running", std::this_thread::get_id(), core_id, start_time, ""});

//     }

//     std::ofstream outfile(filename);
//     for (int i = 0; i < 100; ++i) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
//         std::string timestamp = get_timestamp();
//         outfile << "(" << timestamp << ") "
//                 << "Core: " << core_id << " - "
//                 << "\"Hello world from " << filename << "!\"\n";
//     }
//     outfile.close();

//     {
//         std::lock_guard<std::mutex> lock(process_mutex);
//         for (auto& p : processes) {
//             if (p.filename == filename) {
//                 p.status = "Finished";
//                 p.end_time = get_timestamp();
//                 break;
//             }
//         }
//     }
// }


// void start_file_generation() {
//     std::cout << "Generating files using " << num_cores << " cores...\n";
//     std::vector<std::thread> threads;

//     for (int i = 0; i < num_cores; ++i) {
//         threads.emplace_back([i]() {
//             while (true) {
//                 int current_file = file_count.load();
//                 if (current_file >= num_processes) break;
//                 generate_file(i); // core ID remains same
//             }
//         });
//     }

//     for (auto& t : threads) {
//         t.join();
//     }

//     std::cout << "All files generated.\n";
// }

// void new_screen(std::string name) {

//     if (head == nullptr) {
//         head = new Process(name, 1, 50, get_timestamp()); // placeholder values
//         screen_session(*head);
//         return;
//     }

//     Process *current_screen = head;
            
//     while(current_screen != nullptr){

//         if(current_screen.getProcessName() == name){
//             std::cout << "Screen session with name '" << name << "' already exists.\n";
//             system("pause");
//             return;
//         } else if (current_screen->next == nullptr) {

//             Process *new_screen = new Process(name, 1, 50, get_timestamp()); //placeholder values
//             current_screen->next = new_screen;
//             screen_session(*new_screen);
//             return;
            
//         } else {
//             current_screen = current_screen->next;
//         }
//     }

// }

// void find_screen(std::string name) {
//     Process *current_screen = head;

//     while(current_screen != nullptr && current_screen->name != name){
//         current_screen = current_screen->next;
//     }
    
//     if(current_screen == nullptr){
//         std::cout << "Screen session with name '" << name << "' not found.\n";
//         system("pause");
//         return;
//     }

//     screen_session(*current_screen);
    
// }

/// Create (but don’t attach) a screen session for this process.
// void create_process_screen(const std::string& name, int total_lines = 1){
//     std::cout << "  -> Process ID " << name << " created";

//     std::lock_guard<std::mutex> lock(screenListMutex);

//     // if empty, make head
//     if (!head) {
//         head = new Process(name, 0, total_lines, get_timestamp());
//         return;
//     }
//     // else walk to end (avoid duplicates)
//     Process *cur = head;
//     while (cur) {
//         if (cur->name == name) {
//             // already have a session for this name
//             return;
//         }
//         if (!cur->next) break;
//         cur = cur->next;
//     }
//     cur->next = new Process(name, 0, total_lines, get_timestamp());
//     std::cout << "  -> Process ID " << name << " created with Screen\n";
// }


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
        std::cout << "Max Instructions: " << (1ULL << max_ins) << "\n";
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
        std::string name = choice.substr(10); // get process name

        Process* proc = create_new_process(name); // what to do with process pointer???
        proc = nullptr;
        system("pause");
    } else if (choice.rfind("screen -r", 0) == 0){ 
        // find the screen session by name //UPDATE FIND SCREEN
    } else if (choice.rfind("screen -ls", 0) == 0) {
        //     std::cout << "\nActive screen sessions:\n";

//     // If you ever spawn sessions from multiple threads, 
//     // protect head with a mutex:
//     // std::lock_guard<std::mutex> lock(screenListMutex);

//     Process* curr = head;
//     if (!curr) {
//         std::cout << "  (none)\n";
//     } else {
//         while (curr) {
//             std::cout
//                 << "  Name:   "  << curr->name  
//                 << "    Line:   " << curr->current_line 
//                 << "/"        << curr->total_lines
//                 << "    Created: " << curr->timestamp
//                 << "\n";
//             curr = curr->next;
//         }
//     }

//     system("pause");
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

        // only function in the screen session
        // Logic to display process info for *active_session
        // std::cout << "--- Process: " << (*active_session)->name << " ---\n";
        // std::cout << "Instruction: " << (*active_session)->current_line << " / " << (*active_session)->total_lines << "\n";
        // std::cout << "Created: " << (*active_session)->timestamp << "\n";
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

// bool accept_input(std::string choice, Process *current_screen){
//     bool exit = false;

//     //convert choice to lowercase for case-insensitive comparison
//     for (char &c : choice) {
//         c = std::tolower(static_cast<unsigned char>(c));
//     }

//     if (choice == "initialize") {
//         // initialize os env

//         initialize();

//         std::cout << std::endl << std::endl;
//         std::cout << "Initialized configuration: \nCPU Cores: " << num_cpu << "\n";
//         std::cout << "Scheduler: " << scheduler_type << "\n";
//         std::cout << "Quantum Cycles: " << quantumcycles << "\n";
//         std::cout << "Batch Process Frequency: " << batchprocess_freq << "\n";
//         std::cout << "Min Instructions: " << min_ins << "\n";
//         std::cout << "Max Instructions: " << (1ULL << max_ins) << "\n";
//         std::cout << "Delays per Execution: " << delays_perexec << "\n\n";
//         std::cout << "Max Overall Memory: " << max_overall_mem << "\n";
//         std::cout << "Memory per Frame: " << mem_per_frame << "\n";
//         std::cout << "Memory per Process: " << mem_per_proc << "\n\n\n\n";
    
//         if (current_screen) current_screen->current_line++;
//         system("pause");
//     } else if (choice == "scheduler-start") {
//         scheduler_start();
//         if (current_screen) current_screen->current_line++;
//         std::cout << "ending scheduler_start() function\n";
//         Sleep(60);
//         system("pause");
//     } else if (choice == "scheduler-stop") {
//         std::cout << "Scheduler-stop command recognized. Doing something.\n";
        
//         //instead of forcibly joining all worker threads
//         // scheduler_stop();

//         //stop the process generation, but let it finish draining the queue
//         os_scheduler->stopGenerating();

//         if (current_screen) current_screen->current_line++;
//         system("pause");
//     } else if (choice == "report-util") {
//         std::cout << "Report-util command recognized. Doing something.\n";
//         report_util();
//         if (current_screen) current_screen->current_line++;
//         system("pause");
//     } else if (choice == "clear") {
//         std::cout << "Clear command recognized. Doing something.\n";
//         clear_screen();
//         if (current_screen) current_screen->current_line++;
//         system("pause");
//     } else if (choice == "exit") {
//         std::cout << "Exit command recognized. Exiting...\n";
//         exit = true;
//     } else if (choice == "help") {
//         std::cout << "Help command recognized.\n";
//         std::cout << "Available commands:\n";
//         std::cout << "1. initialize - Initialize the OS environment.\n";
//         std::cout << "2. screen - Initialize the screen.\n";
//         std::cout << "3. Scheduler-test - Start the Scheduler test.\n";
//         std::cout << "4. Scheduler-stop - Stop the Scheduler.\n";
//         std::cout << "5. report-util - Report system information and statistics.\n";
//         std::cout << "6. clear - Clear the screen.\n";
//         std::cout << "7. exit - Exit the OS.\n";
//         std::cout << "8. help - Show this help message.\n";
//         if (current_screen) current_screen->current_line++;
//         system("pause");

//     } else if (choice.rfind("screen -s ", 0) == 0) {
//         std::string name = choice.substr(10);
//         new_screen(name);
//         if (current_screen) current_screen->current_line++;
//     } else if (choice.rfind("screen -r ", 0) == 0) {
//         std::string name = choice.substr(10);  // get <name>
//         find_screen(name);
//         if (current_screen) current_screen->current_line++;

//     } else if (choice == "screen -ls") {
//     std::cout << "\nActive screen sessions:\n";

//     // If you ever spawn sessions from multiple threads, 
//     // protect head with a mutex:
//     // std::lock_guard<std::mutex> lock(screenListMutex);

//     Process* curr = head;
//     if (!curr) {
//         std::cout << "  (none)\n";
//     } else {
//         while (curr) {
//             std::cout
//                 << "  Name:   "  << curr->name  
//                 << "    Line:   " << curr->current_line 
//                 << "/"        << curr->total_lines
//                 << "    Created: " << curr->timestamp
//                 << "\n";
//             curr = curr->next;
//         }
//     }

//     system("pause");
//     if (current_screen) current_screen->current_line++;
        
//     } else if (choice == "^g") {
//         std::thread(scheduler_start).detach();
//     } else {
//         std::cout << "Unknown command: " << choice << "\n";
//     }
    
//     clear_screen();
//     return exit;
// }


void menu(){
    clear_screen();
    OSState current = OSState::MAIN_MENU;
    std::string choice;

    Process* active_process = nullptr;

    while(current != OSState::EXITING){

        clear_screen();
        
        if (current == OSState::MAIN_MENU) {
            screen_init();
        } else if (current == OSState::SCREEN_SESSION) {
            // process_screen_init(active_process); // needs function 
        }   


         if (!std::getline(std::cin, choice)) {
            break;
        }

        if(current == OSState::MAIN_MENU){
            accept_main_menu_input(choice, &current, &active_process);
        } else if (current == OSState::SCREEN_SESSION) {

            // display screen session here???? 
            // fml need new ui
            // THINK LATER

            accept_screen_session_input(choice, &current, &active_process);
        }
    }

    if(os_scheduler != nullptr) {
        os_scheduler -> stopGenerating();

        delete os_scheduler;

        Process* current = head;
        while (current != nullptr) {
            Process* next = current->getNext();
            delete current; // Free the memory for the process
            current = next;
        }
    }

    // while(true){
    //     screen_init();
    //     std:getline(std::cin, choice);
    //     bool exit = accept_input(choice, nullptr);
    //     if(exit == true){
    //         break;
    //     }
    // }
}
