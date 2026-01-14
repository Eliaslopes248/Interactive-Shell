#include <cstddef>
#include <unistd.h>
#include <string>
#include <map>
#include <vector>


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
MODE            m_shell_mode           = MODE::INTERACTIVE;
std::map<MODE, std::string> mode_map;
std::string     m_host;
std::string     m_user;
std::string     m_current_directory;

// ------------------------------------------------------------------------------------
// gathering user info
std::string get_mode();
std::string get_host();
std::string get_user();
void        set_cwd(std::string path="");
// stdin methods
void get_prompt(std::string* prompt);
void get_input();
std::vector<std::string> tokenize(std::string s, const char delimiter);         // splits cin line by spaces and quotes strings
std::vector<std::string> group_tokens(const std::vector<std::string>& tokens); // groups the split strings into command groups

public:

shell();
~shell();

// ------------------------------------------------------------------------------------
int run();
void cleanup();
// ------------------------------------------------------------------------------------
void change_mode(const std::string mode); // change from interactive mode to script mode
void print_user(std::string* prompt);                       // displays the shell prompt
};

// ------------------------------------------------------------------------------------
// Commands will be placed into a TREE data structure ordered by command precedence
// ------------------------------------------------------------------------------------
struct command_node {
    std::string     data;
    command_node*   left;
    command_node*   right;
};

/**
    implements a binary tree with basic operations
*/
class command_tree {
public:
// members variables
command_node*   root;
int             size;

// constructor 
command_tree();
command_tree(command_node* root=NULL);

// tree operations
void    printTree();                        // prints tree content in order traversal
void    insert_node(command_node* cmd);     // adds a command node to the tree
int     get_size();                         // returns size of tree
int     get_size(command_node* root);       // count size of given tree
void    inOrderTrav(command_node* root);    // implements in order traversal
void    clearTree();                        // frees node memory
void    make_tree(std::vector<std::string> commands);      // builds tree

private:


};

