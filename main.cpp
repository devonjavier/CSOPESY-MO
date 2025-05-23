#include "header.h"
#include "screen.h"

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

// struct ScreenSession {
//     std::string name;
//     int current_line;
//     int total_lines;
//     std::string timestamp;
//     ScreenSession *next = nullptr;  // linked list

//     // constructor
//     ScreenSession(std::string n, int current_line, int total_lines, std::string timestamp)
//         : name(n), current_line(current_line), total_lines(total_lines), timestamp(timestamp) {}

    
// };

// ScreenSession *head = nullptr;


int main() {
    std::string choice;
    bool OSrunning = true;

    while (OSrunning) {

        screen_init();
        std::getline(std::cin, choice);

        if (choice == "initialize") {
            std::cout << "Initialize command recognized. Doing something.\n";
            initialize();
        } else if (choice == "screen") {
            std::cout << "Screen command recognized. Doing something.\n";
            screen_init();
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
        } 


        else if (choice.rfind("screen -s ", 0) == 0) {
            std::string name = choice.substr(10);
            new_screen(name);
        }
        else if (choice.rfind("screen -r ", 0) == 0) {
            std::string name = choice.substr(10);  // get <name>
            find_screen(name);
        } else {
            std::cout << "Unknown command: " << choice << "\n";
        }


        clear_screen();
    }

    exit_os(0);
    return 0;
}
