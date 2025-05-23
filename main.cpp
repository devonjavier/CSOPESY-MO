#include "os_interface.cpp"

/*
    BALINGIT, JAVIER, RAMOS, JIRO

 * Task: Week 2 - Group Homework - Setting up your OS emulator
    * ✅ Create your first-ever main menu. Use only the C++ built-in libraries.
    * ✅ Provide your ASCII text header “CSOPESY” or a name for your command line emulator.
    * Set up template code that accepts the following commands: initialize, screen, scheduler-test, scheduler-stop, report-util, clear, and exit. Simply print “X command recognized. Doing something.”, where X is the command recognized.
    * For the clear command, once recognized, the screen clears, reprinting the header texts.
    * For the exit command, the application/CLI immediately closes.
 */


int main() {
    menu();
    exit_os(0);
    return 0;
}
