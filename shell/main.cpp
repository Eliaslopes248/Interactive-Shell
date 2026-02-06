#include "./include/shell.h"
#include <csignal>
#include <cstdlib>
#include <iostream>

void handle_sigint(int rc);
// ----------------------------------------------
int main()
{
    /** handlers for SIGINT */
    signal(SIGINT, handle_sigint);

    shell sh;
    sh.run();
    return 0;
}

/** handler for SIGINT */
inline void handle_sigint(
    int rc)
{
    std::cout << "\nExiting shell gracefully :)" << std::endl;
    exit(0);
}