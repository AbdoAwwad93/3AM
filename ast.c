#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// INTERNAL: Global current line from parser (optional, or pass explicitly)
// For simplicity, we'll let parser set it after creation since most nodes are created there.

ASTNode* create_ast_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    node->value = NULL;
    node->left = NULL;
    node->right = NULL;
    node->next = NULL;
    node->condition = NULL;
    node->body = NULL;
    node->otherwise = NULL;
    node->init = NULL;
    node->increment = NULL;
    node->params = NULL;
    node->args = NULL;
    node->expression = NULL;
    node->var_type = NULL;
    node->line = 0;
    node->column = 0;
    
    return node;
}

ASTNode* create_identifier_node(const char* name) {
    ASTNode* node = create_ast_node(AST_IDENTIFIER);
    if (node && name) {
        node->value = (char*)malloc(strlen(name) + 1);
        strcpy(node->value, name);
    }
    return node;
}

ASTNode* create_number_node(const char* value) {
    ASTNode* node = create_ast_node(AST_NUMBER);
    if (node && value) {
        node->value = (char*)malloc(strlen(value) + 1);
        strcpy(node->value, value);
    }
    return node;
}

ASTNode* create_string_node(const char* value) {
    ASTNode* node = create_ast_node(AST_STRING);
    if (node && value) {
        node->value = (char*)malloc(strlen(value) + 1);
        strcpy(node->value, value);
    }
    return node;
}

ASTNode* create_binary_op_node(const char* op, ASTNode* left, ASTNode* right) {
    ASTNode* node = create_ast_node(AST_BINARY_OP);
    if (node) {
        if (op) {
            node->value = (char*)malloc(strlen(op) + 1);
            strcpy(node->value, op);
        }
        node->left = left;
        node->right = right;
        if (left) node->line = left->line;
    }
    return node;
}

ASTNode* create_comparison_op_node(const char* op, ASTNode* left, ASTNode* right) {
    ASTNode* node = create_ast_node(AST_COMPARISON_OP);
    if (node) {
        if (op) {
            node->value = (char*)malloc(strlen(op) + 1);
            strcpy(node->value, op);
        }
        node->left = left;
        node->right = right;
        if (left) node->line = left->line;
    }
    return node;
}

ASTNode* create_function_call_node(const char* name, ASTNode* args) {
    ASTNode* node = create_ast_node(AST_FUNCTION_CALL);
    if (node) {
        if (name) {
            node->value = (char*)malloc(strlen(name) + 1);
            strcpy(node->value, name);
        }
        node->args = args;
    }
    return node;
}

ASTNode* create_variable_decl_node(const char* type, const char* name, ASTNode* init) {
    ASTNode* node = create_ast_node(AST_VARIABLE_DECL);
    if (node) {
        if (type) {
            node->var_type = (char*)malloc(strlen(type) + 1);
            strcpy(node->var_type, type);
        }
        if (name) {
            node->value = (char*)malloc(strlen(name) + 1);
            strcpy(node->value, name);
        }
        node->expression = init;
    }
    return node;
}

ASTNode* create_assignment_node(const char* name, ASTNode* value) {
    ASTNode* node = create_ast_node(AST_ASSIGNMENT);
    if (node) {
        if (name) {
            node->value = (char*)malloc(strlen(name) + 1);
            strcpy(node->value, name);
        }
        node->expression = value;
    }
    return node;
}

ASTNode* create_block_node(ASTNode* statements) {
    ASTNode* node = create_ast_node(AST_BLOCK);
    if (node) {
        node->body = statements;
    }
    return node;
}

ASTNode* create_when_node(ASTNode* condition, ASTNode* body, ASTNode* otherwise) {
    ASTNode* node = create_ast_node(AST_WHEN);
    if (node) {
        node->condition = condition;
        node->body = body;
        node->otherwise = otherwise;
    }
    return node;
}

ASTNode* create_repeat_node(ASTNode* condition, ASTNode* body) {
    ASTNode* node = create_ast_node(AST_REPEAT);
    if (node) {
        node->condition = condition;
        node->body = body;
    }
    return node;
}

ASTNode* create_loop_node(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = create_ast_node(AST_LOOP);
    if (node) {
        node->init = init;
        node->condition = condition;
        node->increment = increment;
        node->body = body;
    }
    return node;
}

ASTNode* create_function_node(const char* return_type, const char* name, ASTNode* params, ASTNode* body) {
    ASTNode* node = create_ast_node(AST_FUNCTION);
    if (node) {
        if (return_type) {
             node->var_type = (char*)malloc(strlen(return_type) + 1);
             strcpy(node->var_type, return_type);
        }
        if (name) {
            node->value = (char*)malloc(strlen(name) + 1);
            strcpy(node->value, name);
        }
        node->params = params;
        node->body = body;
    }
    return node;
}

ASTNode* create_startclock_node(const char* return_type, ASTNode* params, ASTNode* body) {
    ASTNode* node = create_ast_node(AST_STARTCLOCK);
    if (node) {
        if (return_type) {
             node->var_type = (char*)malloc(strlen(return_type) + 1);
             strcpy(node->var_type, return_type);
        }
        node->params = params;
        node->body = body;
    }
    return node;
}

ASTNode* create_tickout_node(ASTNode* expression) {
    ASTNode* node = create_ast_node(AST_TICKOUT);
    if (node) {
        node->expression = expression;
    }
    return node;
}

ASTNode* create_tickin_node(const char* identifier) {
    ASTNode* node = create_ast_node(AST_TICKIN);
    if (node && identifier) {
        node->value = (char*)malloc(strlen(identifier) + 1);
        strcpy(node->value, identifier);
    }
    return node;
}

ASTNode* create_finish_node(ASTNode* expression) {
    ASTNode* node = create_ast_node(AST_FINISH);
    if (node) {
        node->expression = expression;
    }
    return node;
}

ASTNode* create_import_node(const char* module) {
    ASTNode* node = create_ast_node(AST_IMPORT);
    if (node && module) {
        node->value = (char*)malloc(strlen(module) + 1);
        strcpy(node->value, module);
    }
    return node;
}

ASTNode* create_program_node(ASTNode* statements) {
    ASTNode* node = create_ast_node(AST_PROGRAM);
    if (node) {
        node->body = statements;
    }
    return node;
}

ASTNode* create_timeline_node(const char* name) {
    ASTNode* node = create_ast_node(AST_TIMELINE);
    if (node && name) {
        node->value = (char*)malloc(strlen(name) + 1);
        strcpy(node->value, name);
    }
    return node;
}

void append_statement(ASTNode* block, ASTNode* statement) {
    if (!block || !statement) return;
    
    if (!block->body) {
        block->body = statement;
    } else {
        ASTNode* current = block->body;
        while (current->next) {
            current = current->next;
        }
        current->next = statement;
    }
}

void append_parameter(ASTNode* param_list, ASTNode* param) {
    if (!param_list || !param) return;
    
    if (!param_list->params) {
        param_list->params = param;
    } else {
        ASTNode* current = param_list->params;
        while (current->next) {
            current = current->next;
        }
        current->next = param;
    }
}

void append_argument(ASTNode* arg_list, ASTNode* arg) {
    if (!arg_list || !arg) return;
    
    if (!arg_list->args) {
        arg_list->args = arg;
    } else {
        ASTNode* current = arg_list->args;
        while (current->next) {
            current = current->next;
        }
        current->next = arg;
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;
    
    if (node->value) free(node->value);
    if (node->var_type) free(node->var_type);
    
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->next);
    free_ast(node->condition);
    free_ast(node->body);
    free_ast(node->otherwise);
    free_ast(node->init);
    free_ast(node->increment);
    free_ast(node->params);
    free_ast(node->args);
    free_ast(node->expression);
    
    free(node);
}

const char* ast_node_type_name(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM: return "PROGRAM";
        case AST_TIMELINE: return "TIMELINE";
        case AST_VARIABLE_DECL: return "VARIABLE_DECL";
        case AST_ASSIGNMENT: return "ASSIGNMENT";
        case AST_TICKOUT: return "TICKOUT";
        case AST_TICKIN: return "TICKIN";
        case AST_FINISH: return "FINISH";
        case AST_WHEN: return "WHEN";
        case AST_OTHERWISE: return "OTHERWISE";
        case AST_REPEAT: return "REPEAT";
        case AST_LOOP: return "LOOP";
        case AST_BLOCK: return "BLOCK";
        case AST_EXPRESSION_STMT: return "EXPRESSION_STMT";
        case AST_IMPORT: return "IMPORT";
        case AST_FUNCTION: return "FUNCTION";
        case AST_STARTCLOCK: return "STARTCLOCK";
        case AST_BINARY_OP: return "BINARY_OP";
        case AST_COMPARISON_OP: return "COMPARISON_OP";
        case AST_UNARY_OP: return "UNARY_OP";
        case AST_FUNCTION_CALL: return "FUNCTION_CALL";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_NUMBER: return "NUMBER";
        case AST_STRING: return "STRING";
        case AST_GROUP: return "GROUP";
        case AST_PARAMETER: return "PARAMETER";
        case AST_PARAMETER_LIST: return "PARAMETER_LIST";
        default: return "UNKNOWN";
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    printf("%s", ast_node_type_name(node->type));
    
    if (node->value) {
        printf(": %s", node->value);
    }
    if (node->var_type) {
        printf(" (type: %s)", node->var_type);
    }
    if (node->line > 0) {
        printf(" [Line %d]", node->line);
    }
    printf("\n");
    
    if (node->left) print_ast(node->left, indent + 1);
    if (node->right) print_ast(node->right, indent + 1);
    if (node->condition) print_ast(node->condition, indent + 1);
    if (node->body) print_ast(node->body, indent + 1);
    if (node->otherwise) print_ast(node->otherwise, indent + 1);
    if (node->init) print_ast(node->init, indent + 1);
    if (node->increment) print_ast(node->increment, indent + 1);
    if (node->params) print_ast(node->params, indent + 1);
    if (node->args) print_ast(node->args, indent + 1);
    if (node->expression) print_ast(node->expression, indent + 1);
    if (node->next) print_ast(node->next, indent);
}
