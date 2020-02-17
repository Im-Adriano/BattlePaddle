#if defined(_WIN32) || defined(WIN32)

#ifndef MODULES_COMMAND_EXECUTECOMMAND_H_
#define MODULES_COMMAND_EXECUTECOMMAND_H_

#include <string>
#include <memory>

struct CommandResult {
    std::string std_output;
    std::string std_error;
    std::string start_time;
    std::string end_time;
    bool error = false;
};

std::unique_ptr<CommandResult> RunCommand(std::string command, std::string arguments);

#endif 

#endif