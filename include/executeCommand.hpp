#ifndef EXECUTECOMMAND_H
#define EXECUTECOMMAND_H

#if defined(_WIN32) || defined(WIN32)

#include <string>
#include <memory>
#include <stdlib.h>
#include <string.h>

#define MAX_WAIT 5000  // Wait for 5 seconds.  Used in ExecuteCommand
#define BUFLEN 4096  // Used in ExecuteCommand

#include "time.hpp"

struct CommandResult {
    std::string std_output;
    std::string std_error;
    std::string start_time;
    std::string end_time;
    bool error = false;
};

std::unique_ptr<CommandResult> RunCommand(std::string command, std::string arguments);

#else

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string exec(const char *cmd);

#endif
#endif