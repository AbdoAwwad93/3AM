#ifndef AST_H
#define AST_H

#include "token.h"

// AST Node Types
typedef enum {
    // Program structure start node
    AST_PROGRAM,
    // Namespace
    AST_TIMELINE,
    
    // Statements
    AST_VARIABLE_DECL,
    AST_ASSIGNMENT,
    AST_TICKOUT,
    AST_TICKIN,
    AST_FINISH,
    AST_WHEN,
    AST_OTHERWISE,
    AST_REPEAT,
    AST_LOOP,
    AST_BLOCK,
    AST_EXPRESSION_STMT,
    AST_IMPORT,
    
    // Functions
    AST_FUNCTION,
    AST_STARTCLOCK,
    
    // Expressions
    AST_BINARY_OP,
    AST_COMPARISON_OP,
    AST_UNARY_OP,
    AST_FUNCTION_CALL,
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_STRING,
    AST_GROUP,
    
    // Parameters
    AST_PARAMETER,
    AST_PARAMETER_LIST
} ASTNodeType;

// Forward declaration
typedef struct ASTNode ASTNode;

// AST Node structure
struct ASTNode {
    ASTNodeType type;
    char* value;  // For literals, identifiers, operators, etc.
    
    // Children nodes (for tree structure)
    ASTNode* left;
    ASTNode* right;
    ASTNode* next;  // For linked lists (statements, parameters, etc.)
    
    // Additional fields for specific node types
    ASTNode* condition;  // For when, repeat, loop
    ASTNode* body;       // For blocks, functions, loops
    ASTNode* otherwise;  // For when statements
    ASTNode* init;       // For loop initialization
    ASTNode* increment;  // For loop increment
    ASTNode* params;     // For function parameters
    ASTNode* args;       // For function call arguments
    ASTNode* expression; // For various statement types
    
    // Type information (for variable declarations, parameters)
    char* var_type;  // "second", "minute", "moment", "flag"
    
    // Line/column for error reporting (optional)
    int line;
    int column;
};

// Function declarations
ASTNode* create_ast_node(ASTNodeType type);
ASTNode* create_identifier_node(const char* name);
ASTNode* create_number_node(const char* value);
ASTNode* create_string_node(const char* value);
ASTNode* create_binary_op_node(const char* op, ASTNode* left, ASTNode* right);
ASTNode* create_comparison_op_node(const char* op, ASTNode* left, ASTNode* right);
ASTNode* create_function_call_node(const char* name, ASTNode* args);
ASTNode* create_variable_decl_node(const char* type, const char* name, ASTNode* init);
ASTNode* create_assignment_node(const char* name, ASTNode* value);
ASTNode* create_block_node(ASTNode* statements);
ASTNode* create_when_node(ASTNode* condition, ASTNode* body, ASTNode* otherwise);
ASTNode* create_repeat_node(ASTNode* condition, ASTNode* body);
ASTNode* create_loop_node(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* create_function_node(const char* return_type, const char* name, ASTNode* params, ASTNode* body);
ASTNode* create_startclock_node(const char* return_type, ASTNode* params, ASTNode* body);
ASTNode* create_tickout_node(ASTNode* expression);
ASTNode* create_tickin_node(const char* identifier);
ASTNode* create_finish_node(ASTNode* expression);
ASTNode* create_import_node(const char* module);
ASTNode* create_program_node(ASTNode* statements);
ASTNode* create_timeline_node(const char* name);

// Utility functions
void append_statement(ASTNode* block, ASTNode* statement);
void append_parameter(ASTNode* param_list, ASTNode* param);
void append_argument(ASTNode* arg_list, ASTNode* arg);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);

// Parser function to get the root of the parse tree
ASTNode* get_program_root();

#endif

