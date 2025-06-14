#ifndef CONFIG_H
#define CONFIG_H

#include <string>       //std::string
#include <iostream>     //std::ostream (used indirectly by displayConfig)

#include "src/json.hpp" //nlohmann/json header
using json = nlohmann::json;

class config {
private:
    int cores;
    int processes;

    struct configurations {
        int cores;
        int processes;
    };

    configurations read_json_file(const std::string &filename);

public:
    config(const std::string &filename);

    int getCores() const;
    int getProcesses() const;
    void displayConfig() const;
};

#endif