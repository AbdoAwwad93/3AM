#include "semantic.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int strict_finish_in_functions = 1;
static int semantic_errors = 0;
static Type current_function_return_type = TYPE_VOID;
static const char *current_function_name = NULL;

static int check_statement(ASTNode *node);
static Type check_expression(ASTNode *node);

static void report_error(ASTNode *node, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    if (node && node->line > 0) {
        fprintf(stderr, "[SEMANTIC ERROR] Line %d: ", node->line);
    } else {
        fprintf(stderr, "[SEMANTIC ERROR]: ");
    }
    
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    
    semantic_errors++;
}

static Type check_number_type(const char *value) {
    if (!value) {
        report_error(NULL, "Internal error: numeric literal has no value");
        return TYPE_UNKNOWN;
    }
    for (int i = 0; value[i] != '\0'; i++) {
        if (value[i] == '.') {
            return TYPE_FLOAT;
        }
    }
    
    return TYPE_INTEGER;
}

static Type check_expression(ASTNode *node) {
    if (!node) return TYPE_UNKNOWN;
    
    switch (node->type) {
        case AST_NUMBER:
            return check_number_type(node->value);
        
        case AST_STRING:
            return TYPE_STRING;
        
        case AST_IDENTIFIER: {
            Symbol *s = lookup_symbol(node->value);
            
            if (!s) {
                report_error(node, "Identifier '%s' used before declaration", 
                        node->value);
                return TYPE_UNKNOWN;
            }
            
            if (s->kind == KIND_VARIABLE) {
                return s->type;
            }
            
            if (s->kind == KIND_FUNCTION) {
                report_error(node, "Identifier '%s' is a function, cannot be used as a value. "
                        "Did you mean to call it with '%s()'?", 
                        node->value, node->value);
                return TYPE_UNKNOWN;
            }
            
            return TYPE_UNKNOWN;
        }
        
        case AST_FUNCTION_CALL: {
            Symbol *s = lookup_symbol(node->value);
            
            if (!s) {
                report_error(node, "Call to undeclared function '%s'", node->value);
                return TYPE_UNKNOWN;
            }
            
            if (s->kind != KIND_FUNCTION) {
                report_error(node, "'%s' is not a function, cannot be called", node->value);
                return TYPE_UNKNOWN;
            }
            
            ASTNode *arg = node->args;
            int argi = 0;
            
            while (arg || argi < s->param_count) {
                if (!arg && argi < s->param_count) {
                    report_error(node, "Function '%s' expects %d argument(s), but got %d", 
                            node->value, s->param_count, argi);
                    return s->return_type;
                }
                
                if (arg && argi >= s->param_count) {
                    report_error(node, "Function '%s' called with too many arguments "
                            "(expected %d)", node->value, s->param_count);
                    return s->return_type;
                }
                
                Type arg_type = check_expression(arg);
                Type param_type = s->param_types[argi];
                
                if (param_type != TYPE_UNKNOWN && arg_type != TYPE_UNKNOWN) {
                    if (!types_compatible(param_type, arg_type)) {
                        report_error(arg, "Argument %d of function '%s': "
                                "expected %s but got %s", 
                                argi + 1, node->value, 
                                type_to_string(param_type), 
                                type_to_string(arg_type));
                    }
                }
                
                arg = arg->next;
                argi++;
            }
            
            return s->return_type;
        }
        
        case AST_BINARY_OP: {
            Type left_type = check_expression(node->left);
            Type right_type = check_expression(node->right);
            
            if (strcmp(node->value, "+") == 0) {
                if (left_type == TYPE_STRING || right_type == TYPE_STRING) {
                    return TYPE_STRING;
                }
            }
            
            int left_is_numeric = (left_type == TYPE_INTEGER || left_type == TYPE_FLOAT);
            int right_is_numeric = (right_type == TYPE_INTEGER || right_type == TYPE_FLOAT);
            
            if (!left_is_numeric || !right_is_numeric) {
                // Special check for unknown types to avoid cascading errors for undeclared identifiers
                if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) return TYPE_UNKNOWN;

                report_error(node, "Binary operator '%s' requires numeric operands "
                        "(got left=%s, right=%s)", 
                        node->value, 
                        type_to_string(left_type), 
                        type_to_string(right_type));
                return TYPE_UNKNOWN;
            }
            
            if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
                return TYPE_FLOAT;
            }
            return TYPE_INTEGER;
        }
        
        case AST_COMPARISON_OP: {
            Type left_type = check_expression(node->left);
            Type right_type = check_expression(node->right);
            
            int left_is_numeric = (left_type == TYPE_INTEGER || left_type == TYPE_FLOAT);
            int right_is_numeric = (right_type == TYPE_INTEGER || right_type == TYPE_FLOAT);
            
            if (left_is_numeric && right_is_numeric) {
                return TYPE_FLAG;
            }
            
            if (left_type == TYPE_STRING && right_type == TYPE_STRING) {
                if (strcmp(node->value, "==") == 0 || strcmp(node->value, "!=") == 0) {
                    return TYPE_FLAG;
                }
                report_error(node, "Comparison '%s' not supported for strings "
                        "(only == and != are allowed)", node->value);
                return TYPE_UNKNOWN;
            }
            
            report_error(node, "Comparison '%s' requires matching numeric operands "
                    "or string equality; got left=%s, right=%s", 
                    node->value, 
                    type_to_string(left_type), 
                    type_to_string(right_type));
            return TYPE_UNKNOWN;
        }
        
        case AST_GROUP:
            return check_expression(node->expression);
        
        case AST_UNARY_OP:
            return TYPE_UNKNOWN;
        
        default:
            return TYPE_UNKNOWN;
    }
}

static int check_statement(ASTNode *node) {
    if (!node) return 0;
    
    switch (node->type) {
        case AST_PROGRAM: {
            ASTNode *stmt = node->body;
            while (stmt) {
                check_statement(stmt);
                stmt = stmt->next;
            }
            break;
        }
        
        case AST_STARTCLOCK: {
            Type prev_return_type = current_function_return_type;
            const char *prev_func_name = current_function_name;
            
            if (node->var_type) {
                current_function_return_type = make_type_from_string(node->var_type);
            } else {
                current_function_return_type = TYPE_FLOAT; // Default to minute
            }
            
            current_function_name = "startClock";

            enter_scope();
            // Declare parameters
            for (ASTNode *p = node->params; p; p = p->next) {
                declare_variable_symbol(p->value, make_type_from_string(p->var_type), p->line);
            }

            ASTNode *stmt = node->body;
            while (stmt) {
                check_statement(stmt);
                stmt = stmt->next;
            }
            exit_scope();

            current_function_return_type = prev_return_type;
            current_function_name = prev_func_name;
            break;
        }
        
        case AST_VARIABLE_DECL: {
            Type decl_type = make_type_from_string(node->var_type ? node->var_type : "");
            
            if (decl_type == TYPE_UNKNOWN) {
                report_error(node, "Unknown variable type '%s'", 
                        node->var_type ? node->var_type : "NULL");
            }
            
            if (declare_variable_symbol(node->value, decl_type, node->line) != 0) {
                report_error(node, "Redeclaration of variable '%s' in same scope", 
                        node->value);
            }
            
            if (node->expression) {
                Type init_type = check_expression(node->expression);
                
                if (init_type != TYPE_UNKNOWN && decl_type != TYPE_UNKNOWN) {
                    if (!types_compatible(decl_type, init_type)) {
                        if (decl_type == TYPE_INTEGER && init_type == TYPE_FLOAT) {
                            report_error(node, "Cannot assign float (minute) value to "
                                    "integer (second) variable '%s'. "
                                    "Precision would be lost.", node->value);
                        } else {
                            report_error(node, "Type mismatch in initializer for '%s': "
                                    "declared as %s but initializer is %s", 
                                    node->value, 
                                    type_to_string(decl_type), 
                                    type_to_string(init_type));
                        }
                    }
                }
            }
            break;
        }
        
        case AST_ASSIGNMENT: {
            Symbol *s = lookup_symbol(node->value);
            if (!s) {
                report_error(node, "Assignment to undeclared variable '%s'", node->value);
                break;
            }
            
            Type rhs_type = check_expression(node->expression);
            if (rhs_type != TYPE_UNKNOWN && s->type != TYPE_UNKNOWN) {
                if (!types_compatible(s->type, rhs_type)) {
                    report_error(node, "Type mismatch in assignment to '%s'", node->value);
                }
            }
            break;
        }
        
        case AST_FUNCTION: {
            // Function Body Check
            Type return_type = node->var_type ? make_type_from_string(node->var_type) : TYPE_VOID;
            
            Type prev_return_type = current_function_return_type;
            const char *prev_func_name = current_function_name;
            current_function_return_type = return_type;
            current_function_name = node->value;
            
            enter_scope();
            for (ASTNode *p = node->params; p; p = p->next) {
                declare_variable_symbol(p->value, make_type_from_string(p->var_type), p->line);
            }
            
            check_statement(node->body);
            exit_scope();
            
            current_function_return_type = prev_return_type;
            current_function_name = prev_func_name;
            break;
        }
        
        case AST_TICKOUT:
            check_expression(node->expression);
            break;
            
        case AST_TICKIN:
            if (!lookup_symbol(node->value)) {
                report_error(node, "tickin uses unknown identifier '%s'", node->value);
            }
            break;

        case AST_WHEN:
            if (node->condition) check_expression(node->condition);
            enter_scope();
            check_statement(node->body);
            exit_scope();
            if (node->otherwise) {
                enter_scope();
                check_statement(node->otherwise);
                exit_scope();
            }
            break;

        case AST_BLOCK:
        case AST_OTHERWISE:
            enter_scope();
            ASTNode *s = node->body;
            while (s) {
                check_statement(s);
                s = s->next;
            }
            exit_scope();
            break;

        case AST_REPEAT:
        case AST_LOOP:
            enter_scope();
            if (node->init) check_statement(node->init);
            if (node->condition) check_expression(node->condition);
            if (node->increment) check_expression(node->increment);
            check_statement(node->body);
            exit_scope();
            break;

        case AST_FINISH:
            if (node->expression) {
                Type expr_type = check_expression(node->expression);
                if (current_function_return_type != TYPE_UNKNOWN && !types_compatible(current_function_return_type, expr_type)) {
                    report_error(node, "Return type mismatch in function '%s'", current_function_name);
                }
            } else if (current_function_return_type != TYPE_VOID && current_function_return_type != TYPE_UNKNOWN) {
                report_error(node, "Function '%s' expects return value", current_function_name);
            }
            break;

        case AST_EXPRESSION_STMT:
            check_expression(node->expression);
            break;
            
        default:
            break;
    }
    
    return 0;
}

// Pass 1: Global Registration
static void register_globals(ASTNode *node) {
    if (!node) return;
    
    if (node->type == AST_PROGRAM) {
        ASTNode *stmt = node->body;
        while (stmt) {
            register_globals(stmt);
            stmt = stmt->next;
        }
    } else if (node->type == AST_FUNCTION) {
        int param_count = 0;
        for (ASTNode *p = node->params; p; p = p->next) param_count++;
        
        Type *param_types = NULL;
        if (param_count > 0) {
            param_types = (Type*)malloc(sizeof(Type) * param_count);
            int i = 0;
            for (ASTNode *p = node->params; p; p = p->next, ++i) {
                param_types[i] = make_type_from_string(p->var_type);
            }
        }
        
        Type ret = node->var_type ? make_type_from_string(node->var_type) : TYPE_VOID;
        if (declare_function_symbol(node->value, ret, param_types, param_count, node->line) != 0) {
            report_error(node, "Redeclaration of global function '%s'", node->value);
        }
        if (param_types) free(param_types);
    } else if (node->type == AST_STARTCLOCK) {
        // Register startClock as a internal function so it can be 'linked' but it's usually the entry point
        declare_function_symbol("startClock", TYPE_VOID, NULL, 0, node->line);
    }
}

int semantic_check(ASTNode *program_root) {
    semantic_errors = 0;
    sym_init();
    
    // Register built-ins (if any)
    // Pass 1: Hoist all function declarations
    register_globals(program_root);
    
    // Pass 2: Deep check
    check_statement(program_root);
    
    sym_free();
    return semantic_errors;
}
