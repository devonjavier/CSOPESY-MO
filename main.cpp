#include <iostream>
#include <string>
#include <cstdlib> // for system()
#include <thread>  // for future thread-safe tasks
#include <chrono>
#include <conio.h>
#include <ctime>
#include <iomanip> // idk
#include <sstream> // idk
#include <windows.h>

void initialize();
void screen_init();
void scheduler_test();
void scheduler_stop();
void report_util();
void clear_screen();
void exit_os(int status);
void new_screen();
int count_screen(Screen* head);

struct Screen{
    std::string name;
    int current_line; // for logging
    int total_lines; // for logging
    time_t time_created = time(nullptr);

    Screen(const std::string& screenName = "") 
        : name(screenName), current_line(0), total_lines(0),
          time_created(time(nullptr)), next(nullptr) {}

    void declareTime() const {
        std::tm* localTime = std::localtime(&time_created);
        std::ostringstream oss;
        oss << std::put_time(localTime, "%m/%d/%y, %I:%M:%S %p");
    }

    // std::string getCurrentTime() {
    //     return time_created; 
    // }

    void load_screen() const {
        clear_screen();


        // screen input art

        std::cout << "Screen Name : " << name << "\n";
        std::cout << "Time Created: ";
        std::cout << "Enter command her:";

    };

    struct Screen *next; // for linked list
};


/*
    BALINGIT, JAVIER, RAMOS, JIRO

 * Task: Week 2 - Group Homework - Setting up your OS emulator
    * ✅ Create your first-ever main menu. Use only the C++ built-in libraries.
    * ✅ Provide your ASCII text header “CSOPESY” or a name for your command line emulator.
    * Set up template code that accepts the following commands: initialize, screen, scheduler-test, scheduler-stop, report-util, clear, and exit. Simply print “X command recognized. Doing something.”, where X is the command recognized.
    * For the clear command, once recognized, the screen clears, reprinting the header texts.
    * For the exit command, the application/CLI immediately closes.
 */

//TODO: Make everything thread safe


Screen *head = nullptr; // head of the linked list

int main() {
    std::string choice;
    bool OSrunning = true;


    while (OSrunning) {

        screen_init();
        std::getline(std::cin, choice);

        // Tokenize input
        std::istringstream iss(choice);
        std::string command;
        iss >> command;

        if (choice == "initialize") {
            std::cout << "Initialize command recognized. Doing something.\n";
            initialize();
        } else if (choice == "screen") {
            std::cout << "Screen command recognized. Doing something.\n";
            
            std::string option;
            iss >> option;

            if(option == "-s"){
                std::string screen_name;
                iss >> screen_name;

                if(screen_name.empty()){
                    std::cout << "Please provide a screen name.\n";
                    continue;
                } 

                // Create a new screen node
                Screen* new_screen = new Screen(screen_name);

                
                if (head == nullptr) {
                    head = new_screen;
                } else {
                    Screen* temp = head;
                    while (temp->next != nullptr) {
                        temp = temp->next;
                    }
                    temp->next = new_screen;
                }

                std::string new_time = new_screen->getFormattedTime();
                new_time = time(new_time);
                new_screen->time_created = new_time;

                std::cout << "Screen initialized with name: " << new_screen->name << "\n";

                load_screen(new_screen->name);
    
            } else if (option == "-r"){
                std::string screen_name;
                iss >> screen_name;

                load_screen(screen_name);
            } else {
                std::cout << "Invalid screen option. Use -s or -r.\n";
            }

        } else if (choice == "scheduler-test") {
            std::cout << "Scheduler-test command recognized. Doing something.\n";
            scheduler_test();
        } else if (choice == "scheduler-stop") {
            std::cout << "Scheduler-stop command recognized. Doing something.\n";
            scheduler_stop();
        } else if (choice == "report-util") {
            std::cout << "Report-util command recognized. Doing something.\n";
            report_util();
        } else if (choice == "clear") {
            std::cout << "Clear command recognized. Doing something.\n";
            clear_screen();
        } else if (choice == "exit") {
            std::cout << "Exit command recognized. Exiting...\n";
            break;
        } else if (choice == "help") {
            std::cout << "Help command recognized.\n";
            std::cout << "Available commands:\n";
            std::cout << "1. initialize - Initialize the OS environment.\n";
            std::cout << "2. screen - Initialize the screen.\n";
            std::cout << "3. scheduler-test - Start the scheduler test.\n";
            std::cout << "4. scheduler-stop - Stop the scheduler.\n";
            std::cout << "5. report-util - Report system information and statistics.\n";
            std::cout << "6. clear - Clear the screen.\n";
            std::cout << "7. exit - Exit the OS.\n";
            std::cout << "8. help - Show this help message.\n";
        } else {
            std::cout << "Unknown command: " << choice << "\n";
        }

        system("pause");
        clear_screen();
    }

    exit_os(0);
    return 0;
}

int count_screen(Screen* head){
    int n = 0;
    while(head != nullptr){
        n++;
        head = head->next;
    }
    return n;
}

void new_screen(){

    std::cout << "NEW SCREEN!\n";
    
}



// 1. initialize()
//  - sets up the basic hardware environment
//    - ie yung core core na pinakita ni sir neil
//
//    Gemini says:
//    - Essential initializations:
//        - Initializes the Global Descriptor Table (GDT) or Interrupt Descriptor Table (IDT) if needed for your architecture.
//        - Sets up basic memory management (e.g., identity mapping).
//        - Initializes the screen (in text mode or graphics mode).
//        - Sets up the initial stack.
//    - It does NOT handle full-fledged memory management or process scheduling.  Those come later.

void initialize() {
        // Gemini example:
        // Initialize GDT (or IDT) - architecture-specific!
        // gdt_init();
        // idt_init();

        // Set up a minimal stack (architecture-specific!)
        // stack_init();

        // Basic memory setup (e.g., identity mapping)
        // memory_init();
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

// 3. scheduler_test()
//    - set up a test environment for the scheduler
//    - create several test processes or threads
//    - according to the zoom kanina, each test process/thread will do some simple task
//
//    - starts the scheduler
//
void scheduler_test() {
    // TODO: Need randomization of threads 
    // TODO: Process creation itself
    // create_process("test_process_1", test_process_1_function, PRIORITY_HIGH);
    // create_process("test_process_2", test_process_2_function, PRIORITY_LOW);
    // create_process("test_process_3", test_process_3_function, PRIORITY_MEDIUM);
    // start_scheduler();
    std::cout << "Starting scheduler test... (simulated)\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 4. scheduler_stop()
//    - stops scheduler
//        - preventing context switches
//        - disabling interrupts
//        - setting the CPU to a known state
void scheduler_stop() {
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
    std::cout << "Memory Usage: __ / __ KB\n";
    std::cout << "CPU Usage: __%\n";
    std::cout << "Running Processes:\n";
    // Placeholder for process list
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
//        - stop scheduler
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
