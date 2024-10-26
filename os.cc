#include "os.hpp"

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32

#include <string>
#include <windows.h>

#define WINDOWS_OS

static int spwanProcessWindows(const char* program, char* const args[]) {

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

    // Build the command line string
    std::string commandLine = program;
    for (int i = 1; args[i] != nullptr; ++i) {
        commandLine += " ";
        commandLine += args[i];
    }

    // Documentation: https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
    const int status = !CreateProcess(nullptr,
                                      const_cast<char*>(commandLine.c_str()),
                                      nullptr,
                                      nullptr,
                                      FALSE,
                                      0,
                                      nullptr,
                                      nullptr,
                                      &si,
                                      &pi);
    if(!status) {
        std::fprintf(stderr, "CreateProcess failed: %s\n", GetLastError());
        std::exit(EXIT_FAILURE);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exitCode;
}

#else

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

static int spawnProcessPosix(const char* program, char* const args[]) {

    const pid_t child = fork();

    if(child < 0) {
        std::fputs("An error occured during process creation.", stderr);
        std::perror("fork()");
        std::exit(EXIT_FAILURE);
    }

    if(child == 0) {
        execvp(program, args);

        std::perror("execvp()");
        std::exit(EXIT_FAILURE);
    }

    int status;
    if(waitpid(child, &status, 0) == -1) {
        std::perror("waitpid()");
        std::exit(EXIT_FAILURE);
    }

    return WIFEXITED(status)
        ? WEXITSTATUS(status)
        : -1;
}

#endif

namespace pl0::os {

int spawnProcess(const char* program, char* const args[]) {

#ifdef WINDOWS_OS
    return spawnProcessWindows(program, args);
    #undef WINDOWS_OS
#else
    return spawnProcessPosix(program, args);
#endif
}


}
