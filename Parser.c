#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "ast.h"

extern Token tokens[];
extern int token_count;

int current = 0;
ASTNode* program_root = NULL;

// Get the root of the parse tree
ASTNode* get_program_root() {
    return program_root;
}

Token* peek() {
    if (current >= token_count) return NULL;
    return &tokens[current];
}

Token* advance() {
    if (current < token_count) current++;
    return &tokens[current - 1];
}

int match(TokenType type, const char* value) {
    if (current >= token_count) return 0;
    if (tokens[current].type == type &&
        (value == NULL || strcmp(tokens[current].value, value) == 0)) {
        current++;
        return 1;
    }
    return 0;
}

int check(TokenType type, const char* value) {
    if (current >= token_count) return 0;
    return tokens[current].type == type &&
           (value == NULL || strcmp(tokens[current].value, value) == 0);
}

// Forward declarations
ASTNode* parse_expression();
ASTNode* parse_term();
ASTNode* parse_factor();
ASTNode* parse_statement();
ASTNode* parse_block();
ASTNode* parse_otherwise();
ASTNode* parse_when();
ASTNode* parse_startClock();
ASTNode* parse_function();
ASTNode* parse_import();
ASTNode* parse_variable_declaration();
ASTNode* parse_tickout();
ASTNode* parse_tickin();
ASTNode* parse_finish();
ASTNode* parse_repeat();
ASTNode* parse_loop();
ASTNode* parse_parameter_list();

// Expression parsing (handles arithmetic and comparisons)
ASTNode* parse_factor() {
    if (match(TOKEN_NUMBER, NULL)) {
        return create_number_node(tokens[current - 1].value);
    }
    else if (match(TOKEN_STRING, NULL)) {
        return create_string_node(tokens[current - 1].value);
    }
    else if (match(TOKEN_IDENTIFIER, NULL)) {
        char* name = tokens[current - 1].value;
        // Check if it's a function call
        if (check(TOKEN_SYMBOL, "(")) {
            match(TOKEN_SYMBOL, "(");
            ASTNode* first_arg = NULL;
            ASTNode* last_arg = NULL;
            if (!check(TOKEN_SYMBOL, ")")) {
                first_arg = parse_expression();
                last_arg = first_arg;
                while (match(TOKEN_SYMBOL, ",")) {
                    ASTNode* next_arg = parse_expression();
                    last_arg->next = next_arg;
                    last_arg = next_arg;
                }
            }
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' in function call\n");
            }
            return create_function_call_node(name, first_arg);
        } else {
            return create_identifier_node(name);
        }
    }
    else if (match(TOKEN_SYMBOL, "(")) {
        ASTNode* expr = parse_expression();
        if (!match(TOKEN_SYMBOL, ")")) {
            printf("Error: expected ')'\n");
        }
        ASTNode* group = create_ast_node(AST_GROUP);
        group->expression = expr;
        return group;
    }
    else {
        if (peek()) {
            printf("Error: unexpected token '%s'\n", peek()->value);
            advance();
        }
        return NULL;
    }
}

ASTNode* parse_term() {
    ASTNode* left = parse_factor();
    while (match(TOKEN_SYMBOL, "*") || match(TOKEN_SYMBOL, "/")) {
        char* op = tokens[current - 1].value;
        ASTNode* right = parse_factor();
        left = create_binary_op_node(op, left, right);
    }
    return left;
}

ASTNode* parse_expression() {
    ASTNode* left = parse_term();
    while (match(TOKEN_SYMBOL, "+") || match(TOKEN_SYMBOL, "-")) {
        char* op = tokens[current - 1].value;
        ASTNode* right = parse_term();
        left = create_binary_op_node(op, left, right);
    }
    // Handle comparison operators (check after arithmetic)
    if (current < token_count && tokens[current].type == TOKEN_SYMBOL) {
        const char* op = tokens[current].value;
        if (strcmp(op, ">=") == 0 || strcmp(op, "<=") == 0 ||
            strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
            strcmp(op, ">") == 0 || strcmp(op, "<") == 0) {
            advance();
            ASTNode* right = parse_term();
            left = create_comparison_op_node(op, left, right);
        }
    }
    return left;
}

ASTNode* parse_import() {
    if (match(TOKEN_KEYWORD, "import")) {
        if (match(TOKEN_STRING, NULL)) {
            return create_import_node(tokens[current - 1].value);
        } else {
            printf("Error: expected string after 'import'\n");
            return NULL;
        }
    }
    return NULL;
}

// Parse variable declaration
ASTNode* parse_variable_declaration() {
    if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
         match(TOKEN_KEYWORD, "moment") ||
         match(TOKEN_KEYWORD, "flag")) {
        char* type = tokens[current - 1].value;
        if (match(TOKEN_IDENTIFIER, NULL)) {
            char* name = tokens[current - 1].value;
            ASTNode* init = NULL;
            if (match(TOKEN_SYMBOL, "=")) {
                init = parse_expression();
            }
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after variable declaration\n");
            }
            return create_variable_decl_node(type, name, init);
        } else {
            printf("Error: expected identifier\n");
            return NULL;
        }
    }
    return NULL;
}

// Parse tickout (output) statement
ASTNode* parse_tickout() {
    if (match(TOKEN_KEYWORD, "tickout")) {
        ASTNode* expr = parse_expression();
        if (!match(TOKEN_SYMBOL, ";")) {
            printf("Error: expected ';' after tickout\n");
        }
        return create_tickout_node(expr);
    }
    return NULL;
}

// Parse tickin (input) statement
ASTNode* parse_tickin() {
    if (match(TOKEN_KEYWORD, "tickin")) {
        if (match(TOKEN_IDENTIFIER, NULL)) {
            char* name = tokens[current - 1].value;
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after tickin\n");
            }
            return create_tickin_node(name);
        } else {
            printf("Error: expected identifier after tickin\n");
            return NULL;
        }
    }
    return NULL;
}

// Parse finish (return) statement
ASTNode* parse_finish() {
    if (match(TOKEN_KEYWORD, "finish")) {
        ASTNode* expr = NULL;
        if (!check(TOKEN_SYMBOL, ";")) {
            expr = parse_expression();
        }
        if (!match(TOKEN_SYMBOL, ";")) {
            printf("Error: expected ';' after finish\n");
        }
        return create_finish_node(expr);
    }
    return NULL;
}

ASTNode* parse_when() {
    if (match(TOKEN_KEYWORD, "when")) {
        ASTNode* condition = NULL;
        if (match(TOKEN_SYMBOL, "(")) {
            condition = parse_expression();
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after when condition\n");
            }
        }
        ASTNode* body = NULL;
        if (match(TOKEN_SYMBOL, "{")) {
            body = parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after when block\n");
            }
        }
        ASTNode* otherwise = NULL;
        // Check for otherwise
        if (check(TOKEN_KEYWORD, "otherwise")) {
            otherwise = parse_otherwise();
        }
        return create_when_node(condition, body, otherwise);
    }
    return NULL;
}

ASTNode* parse_otherwise() {
    if (match(TOKEN_KEYWORD, "otherwise")) {
        ASTNode* body = NULL;
        if (match(TOKEN_SYMBOL, "{")) {
            body = parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after otherwise block\n");
            }
        }
        ASTNode* node = create_ast_node(AST_OTHERWISE);
        node->body = body;
        return node;
    }
    return NULL;
}

// Parse repeat (while) loop
ASTNode* parse_repeat() {
    if (match(TOKEN_KEYWORD, "repeat")) {
        ASTNode* condition = NULL;
        if (match(TOKEN_SYMBOL, "(")) {
            condition = parse_expression();
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after repeat condition\n");
            }
        }
        ASTNode* body = NULL;
        if (match(TOKEN_SYMBOL, "{")) {
            body = parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after repeat block\n");
            }
        }
        return create_repeat_node(condition, body);
    }
    return NULL;
}

// Parse loop (for) loop
ASTNode* parse_loop() {
    if (match(TOKEN_KEYWORD, "loop")) {
        ASTNode* init = NULL;
        ASTNode* condition = NULL;
        ASTNode* increment = NULL;
        
        if (match(TOKEN_SYMBOL, "(")) {
            // Parse initialization (type identifier = value;)
            if (!check(TOKEN_SYMBOL, ";")) {
                if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
                    match(TOKEN_KEYWORD, "moment") ||
                    match(TOKEN_KEYWORD, "flag")) {
                    char* type = tokens[current - 1].value;
                    if (match(TOKEN_IDENTIFIER, NULL)) {
                        char* name = tokens[current - 1].value;
                        ASTNode* init_expr = NULL;
                        if (match(TOKEN_SYMBOL, "=")) {
                            init_expr = parse_expression();
                        }
                        init = create_variable_decl_node(type, name, init_expr);
                    }
                }
            }
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after loop initialization\n");
            }
            // Parse condition
            if (!check(TOKEN_SYMBOL, ";")) {
                condition = parse_expression();
            }
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' in loop condition\n");
            }
            // Parse increment (handle i++ or i = i + 1 style)
            if (!check(TOKEN_SYMBOL, ")")) {
                // Check for postfix increment (identifier++)
                if (peek()->type == TOKEN_IDENTIFIER && 
                    current + 1 < token_count &&
                    tokens[current + 1].type == TOKEN_SYMBOL &&
                    strcmp(tokens[current + 1].value, "+") == 0 &&
                    current + 2 < token_count &&
                    tokens[current + 2].type == TOKEN_SYMBOL &&
                    strcmp(tokens[current + 2].value, "+") == 0) {
                    advance(); // identifier
                    char* name = tokens[current - 1].value;
                    advance(); // +
                    advance(); // +
                    // Create a postfix increment node (simplified as assignment)
                    ASTNode* id = create_identifier_node(name);
                    ASTNode* one = create_number_node("1");
                    increment = create_binary_op_node("+", id, one);
                } else {
                    increment = parse_expression();
                }
            }
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after loop header\n");
            }
        }
        ASTNode* body = NULL;
        if (match(TOKEN_SYMBOL, "{")) {
            body = parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after loop block\n");
            }
        }
        return create_loop_node(init, condition, increment, body);
    }
    return NULL;
}


ASTNode* parse_block() {
    ASTNode* block = create_block_node(NULL);
    ASTNode* first_stmt = NULL;
    ASTNode* last_stmt = NULL;
    
    while (peek() && peek()->type != TOKEN_EOF && 
           !check(TOKEN_SYMBOL, "}")) {
        ASTNode* stmt = parse_statement();
        if (stmt) {
            if (!first_stmt) {
                first_stmt = stmt;
                last_stmt = stmt;
            } else {
                last_stmt->next = stmt;
                last_stmt = stmt;
            }
        }
    }
    
    block->body = first_stmt;
    return block;
}


ASTNode* parse_statement() {
    if (current >= token_count) return NULL;
    
    if (peek()->type == TOKEN_COMMENT) {
        advance();
        return NULL;
    }
    
    if (check(TOKEN_KEYWORD, "import")) {
        return parse_import();
    }
    
    if (check(TOKEN_KEYWORD, "second") || check(TOKEN_KEYWORD, "minute") ||
         check(TOKEN_KEYWORD, "moment") ||
        check(TOKEN_KEYWORD, "flag")) {
        return parse_variable_declaration();
    }
    
    if (check(TOKEN_KEYWORD, "tickout")) {
        return parse_tickout();
    }
    
    if (check(TOKEN_KEYWORD, "tickin")) {
        return parse_tickin();
    }
    
    if (check(TOKEN_KEYWORD, "finish")) {
        return parse_finish();
    }
    
    if (check(TOKEN_KEYWORD, "when")) {
        return parse_when();
    }
 
    if (check(TOKEN_KEYWORD, "repeat")) {
        return parse_repeat();
    }
    
    if (check(TOKEN_KEYWORD, "loop")) {
        return parse_loop();
    }
    
    if (peek()->type == TOKEN_IDENTIFIER) {
        Token* ident = peek();
        advance();
        if (check(TOKEN_SYMBOL, "=")) {
            advance(); 
            ASTNode* expr = parse_expression();
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after assignment\n");
            }
            return create_assignment_node(ident->value, expr);
        } else {
            current--; 
        }
    }
    
    ASTNode* expr = parse_expression();
    if (match(TOKEN_SYMBOL, ";")) {
        ASTNode* stmt = create_ast_node(AST_EXPRESSION_STMT);
        stmt->expression = expr;
        return stmt;
    }
    return expr;
}


ASTNode* parse_parameter_list() {
    ASTNode* first_param = NULL;
    ASTNode* last_param = NULL;
    
    if (!check(TOKEN_SYMBOL, ")")) {
        if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
            match(TOKEN_KEYWORD, "moment") ||
            match(TOKEN_KEYWORD, "flag")) {
            char* type = tokens[current - 1].value;
            char* name = NULL;
            if (match(TOKEN_IDENTIFIER, NULL)) {
                name = tokens[current - 1].value;
            }
            ASTNode* param = create_ast_node(AST_PARAMETER);
            if (type) {
                param->var_type = (char*)malloc(strlen(type) + 1);
                strcpy(param->var_type, type);
            }
            if (name) {
                param->value = (char*)malloc(strlen(name) + 1);
                strcpy(param->value, name);
            }
            first_param = param;
            last_param = param;
            
            while (match(TOKEN_SYMBOL, ",")) {
                if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
                    match(TOKEN_KEYWORD, "moment") ||
                    match(TOKEN_KEYWORD, "flag")) {
                    type = tokens[current - 1].value;
                    name = NULL;
                    if (match(TOKEN_IDENTIFIER, NULL)) {
                        name = tokens[current - 1].value;
                    }
                    param = create_ast_node(AST_PARAMETER);
                    if (type) {
                        param->var_type = (char*)malloc(strlen(type) + 1);
                        strcpy(param->var_type, type);
                    }
                    if (name) {
                        param->value = (char*)malloc(strlen(name) + 1);
                        strcpy(param->value, name);
                    }
                    last_param->next = param;
                    last_param = param;
                }
            }
        }
    }
    return first_param;
}

ASTNode* parse_startClock() {
    if (match(TOKEN_KEYWORD, "startClock")) {
        ASTNode* params = NULL;
        if (match(TOKEN_SYMBOL, "(")) {
            params = parse_parameter_list();
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after startClock parameters\n");
            }
            ASTNode* body = NULL;
            if (match(TOKEN_SYMBOL, "{")) {
                body = parse_block();
                if (!match(TOKEN_SYMBOL, "}")) {
                    printf("Error: expected '}' after startClock body\n");
                }
            } else {
                printf("Error: expected '{' after startClock()\n");
            }
            return create_startclock_node(params, body);
        } else {
            printf("Error: expected '(' after startClock\n");
            return NULL;
        }
    }
    return NULL;
}

ASTNode* parse_function() {
    if (match(TOKEN_KEYWORD, "schedule")) {
        if (match(TOKEN_IDENTIFIER, NULL)) {
            char* name = tokens[current - 1].value;
            ASTNode* params = NULL;
            if (match(TOKEN_SYMBOL, "(")) {
                params = parse_parameter_list();
                if (!match(TOKEN_SYMBOL, ")")) {
                    printf("Error: expected ')' after function parameters\n");
                }
            }
            ASTNode* body = NULL;
            if (match(TOKEN_SYMBOL, "{")) {
                body = parse_block();
                if (!match(TOKEN_SYMBOL, "}")) {
                    printf("Error: expected '}' after function body\n");
                }
            }
            return create_function_node(name, params, body);
        }
    }
    return NULL;
}


ASTNode* parse_program() {
    current = 0;
    ASTNode* program = create_program_node(NULL);
    ASTNode* first_stmt = NULL;
    ASTNode* last_stmt = NULL;
    
    while (peek() && peek()->type != TOKEN_EOF) {
        if (peek()->type == TOKEN_COMMENT) {
            advance();
            continue;
        }
        
        if (check(TOKEN_KEYWORD, "timeline")) {
            advance();
            if (match(TOKEN_IDENTIFIER, NULL)) {
                ASTNode* timeline = create_timeline_node(tokens[current - 1].value);
                if (!first_stmt) {
                    first_stmt = timeline;
                    last_stmt = timeline;
                } else {
                    last_stmt->next = timeline;
                    last_stmt = timeline;
                }
            }
            continue;
        }
        
        if (check(TOKEN_KEYWORD, "startClock")) {
            ASTNode* startclock = parse_startClock();
            if (startclock) {
                if (!first_stmt) {
                    first_stmt = startclock;
                    last_stmt = startclock;
                } else {
                    last_stmt->next = startclock;
                    last_stmt = startclock;
                }
            }
            continue;
        }
        
        if (check(TOKEN_KEYWORD, "schedule")) {
            ASTNode* func = parse_function();
            if (func) {
                if (!first_stmt) {
                    first_stmt = func;
                    last_stmt = func;
                } else {
                    last_stmt->next = func;
                    last_stmt = func;
                }
            }
            continue;
        }

        ASTNode* stmt = parse_statement();
        if (stmt) {
            if (!first_stmt) {
                first_stmt = stmt;
                last_stmt = stmt;
            } else {
                last_stmt->next = stmt;
                last_stmt = stmt;
            }
        }
    }
    
    program->body = first_stmt;
    program_root = program;
    return program;
}
