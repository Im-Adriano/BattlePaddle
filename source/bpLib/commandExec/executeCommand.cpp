#include "executeCommand.hpp"

#if defined(_WIN32) || defined(WIN32)

std::unique_ptr<CommandResult> RunCommand(std::string command, std::string arguments) {
    auto result = std::make_unique<CommandResult>();
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
    HANDLE child_std_out_rd = nullptr;  // Handle for StdOut of new process
    HANDLE child_std_out_wr = nullptr;
    HANDLE child_std_err_rd = nullptr;  // Handle for StdErr of new process
    HANDLE child_std_err_wr = nullptr;

    // Handle of parent's StdOut so we can write directly to console
    HANDLE parentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    bool bSuccess = false;

    SECURITY_ATTRIBUTES sa_attr;
    sa_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa_attr.bInheritHandle = TRUE;
    sa_attr.lpSecurityDescriptor = nullptr;

    // Create pipe for the child process Std Out
    if (!CreatePipe(&child_std_out_rd, &child_std_out_wr, &sa_attr, 0))
        printf("CreatePipe StdOut failed (%lu).\n", GetLastError());

    // Make sure the read handle to the pipe for Std Out is NOT inherited
    if (!SetHandleInformation(child_std_out_rd, HANDLE_FLAG_INHERIT, 0))
        printf("SetHandleInformation failed (%lu).\n", GetLastError());

    // Create pipe for the child process Std Err
    if (!CreatePipe(&child_std_err_rd, &child_std_err_wr, &sa_attr, 0))
        printf("CreatePipe StdErr failed (%lu).\n", GetLastError());

    // Make sure the read handle to the pipe for Std Err is NOT inherited
    if (!SetHandleInformation(child_std_err_rd, HANDLE_FLAG_INHERIT, 0))
        printf("SetHandleInformation failed (%lu).\n", GetLastError());

    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = child_std_out_wr;  // Pass standard output from process to handle
    si.hStdError = child_std_err_wr;  // Pass standard error from process to handle
    si.dwFlags |= STARTF_USESTDHANDLES;

    const uint64_t start_time = GetTimeOfDay();

    bSuccess = CreateProcessW(nullptr,  // Name of executable (CreateProcess finds the path)
                              wcommand,  // No Args
                              nullptr,  // Process Handle not inheritable
                              nullptr,  // Thread Handle not inheritable
                              TRUE,  // Set handle inheritance to TRUE, tells Windows to attached the specified handles from the si struct
                              0,  // No Creation flags
                              nullptr,  // Use parent's environment
                              nullptr,  // Use parent's working directory
                              &si,  // Pointer to STARTUPINFO struct
                              &pi);  // Pointer to PROCESS_INFORMATION struct

    WaitForSingleObject(pi.hProcess, MAX_WAIT);
    const uint64_t end_time = GetTimeOfDay();

    std::string child_out;
    std::string child_err;

    if (!bSuccess) {
        printf("CreateProcess failed (%lu).\n", GetLastError());
        child_err = "CreateProcess failed";
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] wcommand;

    // Pipe error resolved with the following documentation
    // https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx
    CloseHandle(child_std_out_wr);
    CloseHandle(child_std_err_wr);

    DWORD dwRead;  // Don't need dwWritten due to not writing to a stream
    CHAR chBuf[BUFLEN];
    const size_t size_ch_buf = sizeof(chBuf);
    ZeroMemory(&chBuf, size_ch_buf);

    // Read StdOut from child process
    bSuccess = false;
    for (;;) {
        bSuccess = ReadFile(child_std_out_rd, chBuf, BUFLEN, &dwRead, nullptr);
        if (!bSuccess || dwRead == 0)
            break;  // Break if there's nothing left, ReadFile will probably return error 109 (ERROR_BROKEN_PIPE)

        child_out.append(chBuf, dwRead);
    }

    // Read StdErr from child process
    bSuccess = false;
    for (;;) {
        bSuccess = ReadFile(child_std_err_rd, chBuf, BUFLEN, &dwRead, nullptr);
        if (!bSuccess || dwRead == 0)
            break;

        child_err.append(chBuf, dwRead);
    }

    result->std_output = child_out;
    result->std_error = child_err;
    result->start_time = std::to_string(start_time);
    result->end_time = std::to_string(end_time);

    // Make Command Response Object to return
    if (child_err != "")
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