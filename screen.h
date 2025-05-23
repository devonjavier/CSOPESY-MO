#include <string>
#include <chrono>
#include <iostream>

struct ScreenSession {
    std::string name;
    int current_line;
    int total_lines;
    std::string timestamp;
    ScreenSession *next = nullptr;  // linked list

    // constructor
    ScreenSession(std::string n, int current_line, int total_lines, std::string timestamp)
        : name(n), current_line(current_line), total_lines(total_lines), timestamp(timestamp) {}
};

ScreenSession *head = nullptr;

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", std::localtime(&now_time));
    return std::string(buffer);
}

void screen_session(ScreenSession& session) {
    std::string command;
    while (true) {
        #ifdef _WIN32
            std::system("cls");
        #else
            std::system("clear");
        #endif
        std::cout << "--- Process: " << session.name << " ---\n";
        std::cout << "Instruction: " << session.current_line << " / " << session.total_lines << "\n";
        std::cout << "Created: " << session.timestamp << "\n";
        std::cout << "\nType 'exit' to return to main menu\n> ";
        std::getline(std::cin, command);
        if (command == "exit") break;

        // Simulate instruction progression
        session.current_line = std::min(session.current_line + 1, session.total_lines);
    }
}

void new_screen(std::string name) {

    if (head == nullptr) {
        head = new ScreenSession(name, 1, 50, get_timestamp()); // placeholder values
        screen_session(*head);
        return;
    }

    ScreenSession *current_screen = head;
            
    while(current_screen != nullptr){

        if(current_screen->name == name){
            std::cout << "Screen session with name '" << name << "' already exists.\n";
            system("pause");
            return;
        } else if (current_screen->next == nullptr) {

            ScreenSession *new_screen = new ScreenSession(name, 1, 50, get_timestamp()); //placeholder values
            current_screen->next = new_screen;
            screen_session(*new_screen);
             // START OF CHANGE CENTRALIZED INPUT TAKER TYPE SHII
            return;
            
        } else {
            current_screen = current_screen->next;
        }
    }
    

}

void find_screen(std::string name) {
    ScreenSession *current_screen = head;

    while(current_screen->name != name){
        current_screen = current_screen->next;
    }
    
    if(current_screen == nullptr){
        std::cout << "Screen session with name '" << name << "' not found.\n";
        return;
    }

    screen_session(*current_screen);
    
}
