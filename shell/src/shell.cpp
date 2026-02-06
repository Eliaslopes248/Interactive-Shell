#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <iostream>
#include "../include/shell.h"
#include <limits.h> 
#include <ostream>
#include <fstream>
#include <string>
#include <sys/_types/_pid_t.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <vector>
#include <set>
#include <stdio.h>
#include <sys/wait.h>





// testing method definitions
command_node* get_test_cmd_nodes();
// --------------------------------------------------------------------
// shell class implementation
// --------------------------------------------------------------------
#define SH shell


SH::shell()
{
    // set mode & mode map values
    m_shell_mode = MODE::INTERACTIVE;
    mode_map = {
        {MODE::INTERACTIVE, "[Interactive Mode]"}, 
        {MODE::SCRIPT, "[Script Mode]"}
    };

    // set the host name
    m_host = get_host();

    // set user name
    m_user = get_user();

    // get init cwd
    set_cwd();

}
SH::~shell() {}

// ---------------------------------------------------------
std::string SH::get_mode(){ return mode_map[this->m_shell_mode ]; };

std::string SH::get_host()
{
    std::array<char, 255> buf{};
    std::string hostname = "";

    if (gethostname(buf.data(), buf.size()) == 0) 
    {
        // remove trailing ".local" in hostname
        std::string unproccessed_hostname = buf.data();
        // set the hostname
        hostname = unproccessed_hostname.substr(0, unproccessed_hostname.length() - 6);
        
    } else 
    {
        perror("gethostname");
    }

    return hostname;
}

std::string SH::get_user()
{ 
    try {
        struct passwd* pw = getpwuid(getuid());
        return pw ? pw->pw_name : "";
        // set value and return
    } catch (...) 
    {
        std::cout << "[ERROR] When getting username" << std::endl;
        return "placeholder";
    }
}

void SH::set_cwd(
    std::string path)
{
    // handle default cwd
    if (path.size() == 0)
    {
        char* buf = getcwd(nullptr, 0);
        std::string cwd(buf);
        m_current_directory =  buf;

    }else {
        // handle overwritten cwd
        m_current_directory =  path;
    }

}

// ---------------------------------------------------------



inline void SH::welcome() {
    std::cout <<
R"(
 __        __   _                             _____ _     ___    _    ____  
 \ \      / /__| | ___ ___  _ __ ___   ___   | ____| |   |_ _|  / \  / ___| 
  \ \ /\ / / _ \ |/ __/ _ \| '_ ` _ \ / _ \  |  _| | |    | |  / _ \ \___ \ 
   \ V  V /  __/ | (_| (_) | | | | | |  __/  | |___| |___ | | / ___ \ ___) |
    \_/\_/ \___|_|\___\___/|_| |_| |_|\___|  |_____|_____|___/_/   \_\____/ 
)" << std::endl;
}

/** exits shell */
inline void SH::exit_shell()
{
    std::cout << "Exiting Shell :)" << std::endl;
    exit(EXIT_SUCCESS);
}


inline void SH::get_prompt(
            std::string* prompt)
{
    if (!prompt) { return; }
    // make sure values are set
    if (m_host.length() == 0)               { get_host(); }
    if (m_user.length() == 0)               { get_user(); }
    // make sure shell mode is set
    if (m_shell_mode != MODE::INTERACTIVE && m_shell_mode != MODE::SCRIPT)
    { 
        m_shell_mode = MODE::INTERACTIVE; 
    }
    // get cwd
    set_cwd();
    /** set prompt w/ ANSI coloring */
    *prompt = "\e[34m" + m_user + '@' + m_host + "\x1b[0m" + ":~" + m_current_directory + "$ ";
    
}

inline void SH::print_user(
            std::string* prompt)
{
    if (prompt == nullptr) { return; }
    get_prompt(prompt);
    // print out on same line
    std::cout << *prompt;
    return;
}

void SH::cleanup(){}

//------------------------------------------------------------------
std::vector<std::string> SH::tokenize(
    const std::string s, 
    char delimiter)
{
    size_t n = s.size();
    if (n == 0) return {};

    size_t start = 0, end = 0;
    std::vector<std::string> output;

    while (end < n) 
    {
        if (s[end] == '"') {
            end++;
            while (end < n && s[end] != '"') {
                end++;
            }
        }

        if (end < n && s[end] == delimiter) {
            output.push_back(s.substr(start, end - start));
            start = end + 1;
        }

        end++;
    }

    if (start < n) {
        output.push_back(s.substr(start, n - start));
    }

    return output;
}

std::vector<std::string> SH::group_tokens(
        const std::vector<std::string>& tokens)
{
    size_t n = tokens.size();
    std::vector<std::string> output;

    // operators
    std::set<std::string> operators = {"||", "&&", ";", "|", "&"};
    
    // group tokens
    for(int i=0; i<n; i++){
        if (operators.find(tokens[i]) != operators.end()){
            output.push_back(tokens[i]);
            continue;
        }

        // add to group until the next operator
        std::string cmd = "";
        while(i < n && operators.find(tokens[i]) == operators.end())
        { 
            cmd += tokens[i] + " ";
            i++; 
        }
        i--;
        output.push_back(cmd);
    }
    return output;
}

/** executes commands doing in order traversal */
int SH::execute_command_tree(
    const command_tree& cmd_tree)
{
    /** retrun -1 if tree is empty */
    if (cmd_tree.get_size() == 0) { return -1; }
    /** execute nodes in order */
    try{
        int result = cmd_tree.execute_commands(); 
    }catch(...){
        return -1;
    }
    /** return 0 if all pass */
    return 0;
}

inline void SH::get_input()
{
    try{
        // get standard input if in interactive mode
        if (m_shell_mode == MODE::INTERACTIVE)
        {
            std::string cmd;
            // get input line from terminal
            std::getline(std::cin, cmd);
            // split the shell cmds by spaces
            std::vector<std::string> tokens = tokenize(cmd, ' ');
            if (!tokens.empty() && (tokens[0] == "exit" || tokens[0] == "Exit")) exit_shell();
            // turn tokens into command groups
            std::vector<std::string> cmd_strings = group_tokens(tokens);
            // create command tree
            command_tree tree;
            tree.make_tree(cmd_strings);
            /** execute commands in tree */
            execute_command_tree(tree);
            return;
        }else{
            // execute the scripts in the input file
            return;
        }
    }catch(...)
    {
        perror("get_input");
        return;
    }
}

int SH::run()
{
    /** print welcome message */
    welcome();
    // prompt string used in whole shell
    std::string* prompt = new std::string();
    while (true)
    {
        try 
        {
            // continuosly show user the shell prompt
            print_user(prompt);
            
            // get input and execute commands
            get_input();

            std::cout << '\n';

        } catch (...) 
        {
            std::cout << "SHELL INTERRUPT: Closing shell" << std::endl;
            exit(EXIT_FAILURE);
        }

    }
    // free memory
    delete prompt;
    cleanup();
    return 0;
}

// -----------------------------------------------------------
/** TREE: implementation for command tree data structure */

#define CMD_TREE command_tree
//----------------------------------------------------------------
// constructors
CMD_TREE::command_tree(
        command_node* root)
{
    // init fields
    this->root = root;
    this->size = root? get_size(root) : 0; 
}
CMD_TREE::command_tree()
{
    // init fields
    this->root = nullptr;
    this->size = 0; 
}
//----------------------------------------------------------------
// TREE OPERTATIONS
int CMD_TREE::get_size() const
{
    return get_size(this->root);
}
int CMD_TREE::get_size(
        command_node* root) const
{
    if (!root) return 0;
    return 
        1 + get_size(root->left) + get_size(root->right);
}

void CMD_TREE::inOrderTrav(
            command_node* root) const
{
    if (!root) { return; }
    inOrderTrav(root->left);
    std::cout << '[' << root->data << ']' << ' ';
    inOrderTrav(root->right);
}

void CMD_TREE::printTree() const
{
    inOrderTrav(this->root);
}

/** executes all commands */
int CMD_TREE::execute_commands() const 
{
    return inOrder_execution(this->root);
} 

/** does in order traversal to execute each node returns 0 if success -1 if failure */
int CMD_TREE::inOrder_execution(
        command_node* node) const
{
    /** reached past leaf or no commands to run */
    if (!node) return 0;

    /** execute left subtree */
    int left_tree_response = inOrder_execution(node->left);
    /** check current node for operator type  || & ; */
    /** convert command data to char* for strcmp  */
    const char* data = node->data.data();

    /** AND operator requires preceding command to finish successfully before running next cmd */
    if (std::strcmp(data, "&") == 0)
    {
        if (left_tree_response != 0){
            /** unable to run command */
            std::cout << "[ERROR] previous command failed, cannot running command -> " << data << std::endl;
            return -1;
        }
    }
    /** check if node is a leaf then execute */
    int current_node_response = -1;
    if (!node->left && !node->right)
    {
        current_node_response = execute_node(node);
        return current_node_response;
    }
    /** execute right subtree */
    int right_tree_response = inOrder_execution(node->right);

    return 0;
}

/** runs a single command */
int CMD_TREE::execute_node(
    command_node* node) const
{
    if (!node) {
        std::cout << "No node given" << std::endl;
        return 0;
    }
    /** check if command exists */
    const char* data = node->data.data();
    /** TASK: perform exec call with node data and args */
    if (node->data.find("cd ") != std::string::npos){
        std::vector<std::string> args = shell::tokenize(node->data, ' ');
        //for (std::string& s: args) printf("token: %s\n", s.c_str());
        /** change directory */
        if (args.size() > 1){
            const char* c = args[1].c_str();
            if (chdir(c) != 0)
            {
                return 1;
            }
        }else{
            return 1;
        }
        
    }else {
        /** check if command lives on system PATH */
        std::string path;
        bool exists = is_built_in(node->data, path);
        if (!exists){
            printf("Command '%s' isnt recognized", node->data.c_str());
            return -1;
        }
        /** TASK: make exec call */
        int r = node_exec(node->data);
    }


    return 0;
}

 /**  coverts command to a char*[] */
std::vector<char*> CMD_TREE::to_char_ptr_array(
    std::string str) const
{
    /** retokenize the cmd */
    std::vector<std::string> tokens = shell::tokenize(str, ' ');
    /** creates char*[] */
    std::vector<char*> args;

    /** allocate memory and copy strings to ensure they persist */
    for (const auto& s : tokens) {
        char* arg = new char[s.length() + 1];
        std::strcpy(arg, s.c_str());
        args.push_back(arg);
    }
    args.push_back(nullptr);

    return args;
}

/** converts cmd string to char*[], fork() process and runs execpv in child proccess  */
int  CMD_TREE::node_exec(
    std::string command) const
{
    /** convert cmd to char*[] */
    std::vector<char*> args = to_char_ptr_array(command);
    /** create child process */
    pid_t pid = fork();
    /** in child process */
    if (pid == 0)
    {
        execvp(args[0], args.data());
        perror("execvp failed");
        // Clean up allocated memory before exit
        for (char* arg : args) {
            if (arg) delete[] arg;
        }
        _exit(1);
    }else if (pid > 0){
        /** wait for child to be done */
        waitpid(pid, nullptr, 0);
        // Clean up allocated memory in parent
        for (char* arg : args) {
            if (arg) delete[] arg;
        }
    }else{
        // Clean up allocated memory on fork failure
        for (char* arg : args) {
            if (arg) delete[] arg;
        }
        perror("Error when forking proccess");
    }
    return 0;
}

/** constructs a balanced tree of nodes */
command_node* CMD_TREE::to_balanced_tree(
    const std::vector<command_node*>& nodes, 
    int lower, 
    int upper)
{
    // checl if out of bounds
    if (lower > upper) return nullptr;
    // find mid point of current node range
    int mid = lower + (upper - lower) / 2;

    if (mid < 0)                return nullptr;
    if (mid > nodes.size()-1)   return nullptr;

    // set left and right children
    command_node* n = nodes[mid];
    // Safety check for null node
    if (!n) return nullptr;  
    
    n->left         = to_balanced_tree(nodes, lower, mid-1);
    n->right        = to_balanced_tree(nodes, mid+1, upper);
    
    return n;
}

/** turns tokens into nodes and sets the root as a balanced tree */
void CMD_TREE:: make_tree(
    const std::vector<std::string>& commands)
{
    size_t n = commands.size();
    if (n==0) return;
    // reserve n slots for commands
    std::vector<command_node*> nodes;
    nodes.reserve(n);
    // creates nodes
    for (std::string s : commands)
    {
        /** creates node ptr */
        command_node* cmd = new command_node;
        /** set values of node */
        cmd->data = s;
        cmd->left = nullptr;
        cmd->right = nullptr;
        /** add to vector of nodes */
        nodes.push_back(cmd);
    }

    int num_nodes = nodes.size();
    if (num_nodes > 0)
    {
        /** create balances tree of nodes */
        this->root = to_balanced_tree(nodes, 0, num_nodes - 1);
    }

}

/** checks if a command is in users $PATH or not and if not check local for exe */
bool CMD_TREE::is_built_in(
    std::string exe,
    std::string& path
) const
{
    // Trim whitespace and extract just the executable name (first token)
    // Remove leading/trailing whitespace
    size_t trim_start = exe.find_first_not_of(" \t\n\r");
    if (trim_start == std::string::npos) { return false; }
    size_t trim_end = exe.find_last_not_of(" \t\n\r");
    exe = exe.substr(trim_start, trim_end - trim_start + 1);
    
    // Extract just the first word (executable name) if there are arguments
    size_t space_pos = exe.find_first_of(" \t\n\r");
    if (space_pos != std::string::npos) {
        exe = exe.substr(0, space_pos);
    }
    
    /** gets system $PATH */
    std::string path_string = get_path();

    if (path_string.empty()) { return false; }

    /** tokenize by ":" delimeter */
    /** two ptr algo to tokenize the string */
    std::vector<std::string> path_tokens;
    int end = 0, start = 0, n = path_string.size();

    /** splits the path string by ':' */
    while (end < n)
    {
        if (end == n || path_string[end] == ':')
        {
            std::string sub = path_string.substr(start, (end-start));
            path_tokens.push_back(sub);
            start = end+1;
        }
        end++;
    }

    /** check each path to see if exe exists */
    for(std::string& s : path_tokens)
    {
        /** construct absolute path */
        std::string exe_path = s + '/' + exe;
        bool found = exe_exists(exe_path);
        if (found)
        {
            path = exe_path;
            return true;
        }
    }
    //std::cout << "[ERROR] COMMAND NOT FOUND" << std::endl;
    return false;
}
// -------------------------- TESTING ---------------------------------

command_node* get_test_cmd_nodes()
{
    // create root node
    command_node* root = new command_node;
    root->data = "echo";
    // make two children nodes
    command_node* node1 = new command_node;
    node1->data = "'hello world'";
    command_node* node2 = new command_node;
    node2->data = "nano";

    // set children
    root->left = node1;
    root->right = node2;

    return root;
}

// -------------------------- TESTING ---------------------------------

/** GLOBAL METHODS */

/** gets the system $PATH and to_string() it */
std::string get_path()
{
    char* path;
    /** check path for MAC */
    #ifdef __APPLE__
        path = std::getenv("PATH");
    /** check path for LINUX */
    #elif defined(__linux__) || defined(__unix__)
        path = std::getenv("PATH");
    /** check path for WINDOWS */
    #elif defined(_WIN32)
        path = std::getenv("Path");
    #else
        path = std::getenv("PATH");
    #endif

    /** check path for exe match */
    std::string path_string = "";
    if (path){
        std::string path_string(path);
        return path_string;
    }else{
        return "";
    }
}

/** checks if a executable exists via $PATH token path exe) /bin/ls = true; */
/**
* NOTE: we use std::ifstream because system executables 
*       only allow read access permission so std::fstream
*       wont work
*/ 
bool exe_exists(
    const std::string& path)
{
    if (path.empty()) { return false; }
    /** creates input stream file object */
    std::ifstream f;
    f.open(path, std::ios::binary);
    /** if it exists it will be opened */
    return f.is_open();
}
