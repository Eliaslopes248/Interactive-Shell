#include <cstddef>
#include <unistd.h>
#include <string>
#include <map>
#include <vector>


/** class declarations */
class command_tree;
struct command_node;
class shell;

/** global methods */
std::string get_path();
bool        exe_exists(const std::string& path);

/**
defines mode for shell:
    - interactive mode expects written out scripts
    - script mode expects a path to a file
*/
enum MODE{ INTERACTIVE, SCRIPT };


// ------------------------------------------------------------------------------------
class shell
{
private:
MODE                        m_shell_mode        = MODE::INTERACTIVE;
std::map<MODE, std::string> mode_map;
std::string                 m_host;
std::string                 m_user;
std::string                 m_current_directory;

// ------------------------------------------------------------------------------------
// gathering user info
void                     exit_shell();
std::string              get_mode();
std::string              get_host();
std::string              get_user();
void                     set_cwd(std::string path="");
inline void              get_prompt(std::string* prompt);
inline void              get_input();
inline void              welcome();
//std::vector<std::string> tokenize(std::string s, const char delimiter);         // splits cin line by spaces and quotes strings
std::vector<std::string> group_tokens(const std::vector<std::string>& tokens); // groups the split strings into command groups

public:
static std::vector<std::string> tokenize(std::string s, const char delimiter);         // splits cin line by spaces and quotes strings

shell();
~shell();

// ------------------------------------------------------------------------------------
int             run();
void            cleanup();
void            change_mode(const std::string mode); // change from interactive mode to script mode
inline void     print_user(std::string* prompt);
int             execute_command_tree( const command_tree& tree);                       // displays the shell prompt
};

// ------------------------------------------------------------------------------------
// Commands will be placed into a TREE data structure ordered by command precedence
// ------------------------------------------------------------------------------------
struct command_node 
{
    std::string     data;
    command_node*   left;
    command_node*   right;
};

/**
    implements a binary tree with basic operations
*/
class command_tree 
{


public:
// members variables
command_node*   root;
// constructor 
command_tree();
command_tree(command_node* root);

// tree operations
inline void     printTree() const;                                          // prints tree content in order traversal
void            insert_node(command_node* cmd);                             // adds a command node to the tree
int             get_size() const;                                           // returns size of tree
int             get_size(command_node* root) const;                         // count size of given tree
void            inOrderTrav(command_node* root) const;                      // implements in order traversal
void            clearTree();                                                // frees node memory
void            make_tree(const std::vector<std::string>& commands);        // builds tree by making nddes and calling a build tree method
command_node*   to_balanced_tree(const std::vector<command_node*>& nodes, int lower, int upper); // builds balanced tree of nodes
int             execute_commands() const;                                   // execute all commands in the tree
bool            is_built_in(std::string exe, std::string& path) const;      // checks if a command is in the systems path

private:
int size;

/** in order traversal algo to execute command node by node */
int                 inOrder_execution(command_node* node) const;
int                 execute_node(command_node* node) const;         // executes a single node
int                 node_exec(std::string command)   const ;                 // forks process and runs execpv in child process
std::vector<char*>  to_char_ptr_array(std::string str) const;    // coverts command to a char*[]



};

