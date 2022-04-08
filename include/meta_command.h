#ifndef META_COMMAND_H_
#define META_COMMAND_H_

#include "wxdb_define.h"

#include <unordered_map>
#include <string>

class MetaCommand 
{
public:
    MetaCommand() : command2type_(nullptr), main_loop_(true) { 
        Init();
    }

    ~MetaCommand() = default;

    bool MainLoop() { return main_loop_; }

    void ExecuteMetaCommand(const std::string &userInput) 
    {
        ParseMetaCommand(userInput);
        switch(type_) 
        {
            case META_COMMAND::URECOGNIZED:
                std::cout << "Unknown command " << "\"" << userInput << "\"" << std::endl;
                break;
            case META_COMMAND::EXIT: 
                main_loop_ = false;
                break;
            case META_COMMAND::HELP:
                // TODO walsons 
                break;
        }
    }
private:
    std::unordered_map<std::string, META_COMMAND> *command2type_;
    META_COMMAND type_;
    bool main_loop_;

private:
    void Init() 
    {
        command2type_ = new std::unordered_map<std::string, META_COMMAND>{
            {".exit", META_COMMAND::EXIT},
            {".help", META_COMMAND::HELP}
        };
    }

    void ParseMetaCommand(const std::string &userInput)
    {
        if (command2type_->find(userInput) == command2type_->end()) 
        {
            type_ = META_COMMAND::URECOGNIZED;
        }
        else
        {
            type_ = (*command2type_)[userInput];
        }
    }
};

#endif
