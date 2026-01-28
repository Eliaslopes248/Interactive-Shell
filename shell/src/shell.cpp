#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "../include/shell.h"
#include <limits.h> 
#include <ostream>
#include <string>
#include <sys/_types/_pid_t.h>
#include <unistd.h>
#include <pwd.h>
#include <vector>
#include <set>
#include <stdio.h>
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

void SH::set_cwd(std::string path)
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
void SH::get_prompt(std::string* prompt)
{
    if (!prompt) { return; }
    // make sure values are set
    if (m_current_directory.length() == 0)  { set_cwd(); }
    if (m_host.length() == 0)               { get_host(); }
    if (m_user.length() == 0)               { get_user(); }
    // make sure shell mode is set
    if (m_shell_mode != MODE::INTERACTIVE && m_shell_mode != MODE::SCRIPT)
    { 
        m_shell_mode = MODE::INTERACTIVE; 
    }

    *prompt =  m_user + '@' + m_host + ":~" + m_current_directory + "$ ";
    
}

void SH::print_user(std::string* prompt)
{
    if (prompt == nullptr) { return; }
    // print out the prompt
    std::cout << "--------------------------------------"<< std::endl;
    std::cout << "ELIAS SHELL"<< std::endl;
    std::cout << "--------------------------------------"<< std::endl;
    get_prompt(prompt);
    // print out on same line
    std::cout << *prompt;

}

void SH::cleanup(){}

//------------------------------------------------------------------
std::vector<std::string> SH::tokenize(const std::string s, char delimiter)
{
    size_t n = s.size();
    if (n == 0) return {};

    size_t start = 0, end = 0;
    std::vector<std::string> output;

    while (end < n) {
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

std::vector<std::string> SH::group_tokens(const std::vector<std::string>& tokens)
{
    size_t n = tokens.size();
    std::vector<std::string> output;

    // operators
    std::set<std::string> operators = {"||", "&&", ";", "|", "&"};

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

void SH::get_input(){
    try{
        // get standard input if in interactive mode
        if (m_shell_mode == MODE::INTERACTIVE)
        {
            std::string cmd;
            // get input line from terminal
            std::getline(std::cin, cmd);
            // split the shell cmds by spaces
            std::vector<std::string> tokens = tokenize(cmd, ' ');
            // turn tokens into command groups
            std::vector<std::string> cmd_strings = group_tokens(tokens);
            // create command tree
            command_tree tree;
            tree.make_tree(cmd_strings);

            //exit(0);
        }else{
            // execute the scripts in the input file
        }
    }catch(...)
    {
        perror("get_input");
        return;
    }
}

int SH::run()
{
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

/**
    implementation for command tree data structure
*/

#define CMD_TREE command_tree
//----------------------------------------------------------------
// constructors
CMD_TREE::command_tree(command_node* root)
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
int CMD_TREE::get_size()
{
    return get_size(this->root);
}
int CMD_TREE::get_size(command_node* root)
{
    if (!root) return 0;
    return 
        1 + get_size(root->left) + get_size(root->right);
}

void CMD_TREE::inOrderTrav(command_node* root)
{
    if (!root) { return; }
    inOrderTrav(root->left);
    std::cout << '[' << root->data << ']' << ' ';
    inOrderTrav(root->right);
}

void CMD_TREE::printTree()
{
    inOrderTrav(this->root);
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
void CMD_TREE:: make_tree(std::vector<std::string> commands)
{
    size_t n = commands.size();
    if (n==0) return;
    // reserve n slots for commands
    std::vector<command_node*> nodes;
    nodes.reserve(n);
    // creates nodes
    for (std::string& s : commands)
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

    /** print tree */
    printTree();
    /** DELETE: test is build in  */
    is_built_in("ls");
}

/** checks if a command is in users $PATH or not and if not check local for exe */
bool CMD_TREE::is_built_in(
    std::string exe)
{
    char* path;
    /** check path for MAC */
    #ifdef __APPLE__
        path = std::getenv("PATH");
    /** check path for LINUX */
    #elif defined(__linux__) || defined(__unix__)
        path = std::getenv("Path");
    /** check path for WINDOWS */
    #elif defined(_WIN32)
        path = std::getenv("Path");
    #else
        path = std::getenv("PATH");
    #endif

    /** check path for exe match */
    char* ptr = path;
    while(*ptr != '\0'){
        /** TASK: tokenize $PATH */
        printf("%c", *ptr);
        ptr++;
    }

    /** TASK: see if theres a match */


    return false;
}
// ------------------ TESTING ---------------------------------

command_node* get_test_cmd_nodes(){
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