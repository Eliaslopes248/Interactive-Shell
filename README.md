A C++ Unix-like shell built with CMake that implements a full command execution pipeline. The shell includes input parsing, lexical tokenization, command grouping, PATH-based executable resolution, and process execution using fork, exec, and inter-process communication via pipes.

The shell exposes system context such as username, hostname, and current working directory, and is designed with extensibility in mind, including planned SSH support.


======================================================
            NEXT TASK 
======================================================
- Implement -> bool is_built_in(std::string exe); 
    - gets PATH
    - tokenizes PATH
    - checks for match with exe
- Implement -> int execute_commands();  
    - recursive function does inOrderTrav but returns int
        - SUCCESS=0 FAIL=!0
    - return code can dictate if the next node will get executed or not
