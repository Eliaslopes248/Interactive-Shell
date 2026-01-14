A C++ Unix-like shell built with CMake that implements a full command execution pipeline. The shell includes input parsing, lexical tokenization, command grouping, PATH-based executable resolution, and process execution using fork, exec, and inter-process communication via pipes.

The shell exposes system context such as username, hostname, and current working directory, and is designed with extensibility in mind, including planned SSH support.
