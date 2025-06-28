#include <string>


class ICommand {
public:
    virtual void execute() = 0;
    virtual std::string getType() const = 0;
    virtual ~ICommand() {}
};

// add execute functions
class PRINT : public ICommand{

};

class DECLARE : public ICommand {

};

class ADD : public ICommand {

};

class SUBTRACT : public ICommand {

};

class SLEEP : public ICommand {

};

class FOR : public ICommand {

};