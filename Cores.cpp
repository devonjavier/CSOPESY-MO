#include <iostream>

class Cores {
    private:
        int core_id;
    public:
        Cores(int id) : core_id(id) {

        }

        void displayCores() const {
                std::cout << "Process ID: " << core_id << "\n" << std::endl;
            }
};