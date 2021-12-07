#ifndef META_COMMAND_H_
#define META_COMMAND_H_

#include <unordered_map>
#include <string>

class MetaCommand {
public:
    MetaCommand() : command2number_(nullptr) 
    {
        Init();
    }
    ~MetaCommand() 
    {
        if (command2number_ != nullptr)
        {
            delete command2number_;
            command2number_ = nullptr;
        }
    }
    void Parse(const std::string &userInput, bool &mainLoopFlag) const 
    {
        switch(command2number(userInput)) 
        {
            case META_COMMAND_URECOGNIZED:
            {
                std::cout << "Unknown command " << "\"" << userInput << "\"" << std::endl;
                break;
            }
            case META_COMMAND_EXIT: 
            {
                mainLoopFlag = false;
                break;
            }
            default:
                break;
        }
    }
private:
    enum MetaCommandType
    {
        META_COMMAND_URECOGNIZED,
        META_COMMAND_EXIT
    };
    std::unordered_map<std::string, MetaCommandType> *command2number_;

private:
    void Init() 
    {
        command2number_ = new std::unordered_map<std::string, MetaCommandType>{
            {".exit", META_COMMAND_EXIT}
        };
    }
    // If the command is not found, return -1
    MetaCommandType command2number(const std::string &command) const
    {
        if (command2number_->find(command) == command2number_->end()) {
            return META_COMMAND_URECOGNIZED;
        }
        return (*command2number_)[command];
    }
};

#endif
