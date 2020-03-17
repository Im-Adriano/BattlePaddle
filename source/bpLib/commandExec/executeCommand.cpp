#include "executeCommand.hpp"

#if defined(_WIN32) || defined(WIN32)

std::unique_ptr<CommandResult> RunCommand(std::string command, std::string arguments) {
    auto result = std::make_unique<CommandResult>();
    uint64_t startTime;
    uint64_t endTime;
    bool commandError = false;
    // Get command from Action object

    // IMPORTANT: There may be a better way to do this conversion using const_cast<LPWSTR>
    // https://stackoverflow.com/questions/19554841/how-to-use-const-cast


    // Convert Command to const char* and then convert to wchar_t * (LPTSTR)
    command.append(arguments);
    const char *buffCommand = command.c_str();  // Convert command to char array
    size_t newsize = strlen(buffCommand) + 1;  // Get length of the command C String
    LPWSTR wcommand = new wchar_t[newsize];  // wchar_t *
    size_t convertedChars = 0;

    // Multi-byte to wide str (wchar_t) conversion
    mbstowcs_s(&convertedChars, wcommand, newsize, buffCommand, _TRUNCATE);

    // Must use CreateProcess to capture IO
    STARTUPINFOW si;  // StartupInfo Struct to initialize with the process being created
    PROCESS_INFORMATION pi;  // ProcessInformation struct
    HANDLE childStdOut_Rd = NULL;  // Handle for StdOut of new process
    HANDLE childStdOut_Wr = NULL;
    HANDLE childStdErr_Rd = NULL;  // Handle for StdErr of new process
    HANDLE childStdErr_Wr = NULL;

    // Handle of parent's StdOut so we can write directly to console
    HANDLE parentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    bool bSuccess = false;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create pipe for the child process Std Out
    if (!CreatePipe(&childStdOut_Rd, &childStdOut_Wr, &saAttr, 0))
        printf("CreatePipe StdOut failed (%d).\n", GetLastError());

    // Make sure the read handle to the pipe for Std Out is NOT inherited
    if (!SetHandleInformation(childStdOut_Rd, HANDLE_FLAG_INHERIT, 0))
        printf("SetHandleInformation failed (%d).\n", GetLastError());

    // Create pipe for the child process Std Err
    if (!CreatePipe(&childStdErr_Rd, &childStdErr_Wr, &saAttr, 0))
        printf("CreatePipe StdErr failed (%d).\n", GetLastError());

    // Make sure the read handle to the pipe for Std Err is NOT inherited
    if (!SetHandleInformation(childStdErr_Rd, HANDLE_FLAG_INHERIT, 0))
        printf("SetHandleInformation failed (%d).\n", GetLastError());

    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = childStdOut_Wr;  // Pass standard output from process to handle
    si.hStdError = childStdErr_Wr;  // Pass standard error from process to handle
    si.dwFlags |= STARTF_USESTDHANDLES;

    startTime = GetTimeOfDay();

    bSuccess = CreateProcessW(NULL,  // Name of executable (CreateProcess finds the path)
                              wcommand,  // No Args
                              NULL,  // Process Handle not inheritable
                              NULL,  // Thread Handle not inheritable
                              TRUE,  // Set handle inheritance to TRUE, tells Windows to attached the specified handles from the si struct
                              0,  // No Creation flags
                              NULL,  // Use parent's environment
                              NULL,  // Use parent's working directory
                              &si,  // Pointer to STARTUPINFO struct
                              &pi);  // Pointer to PROCESS_INFORMATION struct

    WaitForSingleObject(pi.hProcess, MAX_WAIT);
    endTime = GetTimeOfDay();

    std::string childOut = "";
    std::string childErr = "";

    if (!bSuccess) {
        printf("CreateProcess failed (%d).\n", GetLastError());
        childErr = "CreateProcess failed";
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete wcommand;

    // Pipe error resolved with the following documentation
    // https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx
    CloseHandle(childStdOut_Wr);
    CloseHandle(childStdErr_Wr);

    DWORD dwRead;  // Don't need dwWritten due to not writing to a stream
    CHAR chBuf[BUFLEN];
    size_t sizeChBuf = sizeof(chBuf);
    ZeroMemory(&chBuf, sizeChBuf);

    // Read StdOut from child process
    bSuccess = false;
    for (;;) {
        bSuccess = ReadFile(childStdOut_Rd, chBuf, BUFLEN, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
            break;  // Break if there's nothing left, ReadFile will probably return error 109 (ERROR_BROKEN_PIPE)

        childOut.append(chBuf, dwRead);
    }

    // Read StdErr from child process
    bSuccess = false;
    for (;;) {
        bSuccess = ReadFile(childStdErr_Rd, chBuf, BUFLEN, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
            break;

        childErr.append(chBuf, dwRead);
    }

    result->std_output = childOut;
    result->std_error = childErr;
    result->start_time = std::to_string(startTime);
    result->end_time = std::to_string(endTime);

    // Make Command Response Object to return
    if (childErr != "")
        result->error = true;

    return result;
}


#else

std::string exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

#endif