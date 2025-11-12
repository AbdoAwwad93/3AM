#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

extern Token tokens[];
extern int token_count;

int current = 0;

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

void parse_expression();
void parse_term();
void parse_factor();
void parse_statement();
void parse_block();
void parse_otherwise();
void parse_when();
void parse_startClock();

// Expression parsing (handles arithmetic and comparisons)
void parse_factor() {
    if (match(TOKEN_NUMBER, NULL)) {
        printf("  Number(%s)\n", tokens[current - 1].value);
    }
    else if (match(TOKEN_STRING, NULL)) {
        printf("  String(\"%s\")\n", tokens[current - 1].value);
    }
    else if (match(TOKEN_IDENTIFIER, NULL)) {
        // Check if it's a function call
        if (check(TOKEN_SYMBOL, "(")) {
            printf("  FunctionCall(%s)\n", tokens[current - 1].value);
            match(TOKEN_SYMBOL, "(");
            if (!check(TOKEN_SYMBOL, ")")) {
                parse_expression();
                while (match(TOKEN_SYMBOL, ",")) {
                    parse_expression();
                }
            }
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' in function call\n");
            }
        } else {
            printf("  Identifier(%s)\n", tokens[current - 1].value);
        }
    }
    else if (match(TOKEN_SYMBOL, "(")) {
        printf("  ExpressionGroup(\n");
        parse_expression();
        if (!match(TOKEN_SYMBOL, ")")) {
            printf("Error: expected ')'\n");
        }
        printf("  )\n");
    }
    else {
        if (peek()) {
            printf("Error: unexpected token '%s'\n", peek()->value);
            advance();
        }
    }
}

void parse_term() {
    parse_factor();
    while (match(TOKEN_SYMBOL, "*") || match(TOKEN_SYMBOL, "/")) {
        printf("    Operator(%s)\n", tokens[current - 1].value);
        parse_factor();
    }
}

void parse_expression() {
    parse_term();
    while (match(TOKEN_SYMBOL, "+") || match(TOKEN_SYMBOL, "-")) {
        printf("    Operator(%s)\n", tokens[current - 1].value);
        parse_term();
    }
    // Handle comparison operators (check after arithmetic)
    if (current < token_count && tokens[current].type == TOKEN_SYMBOL) {
        const char* op = tokens[current].value;
        if (strcmp(op, ">=") == 0 || strcmp(op, "<=") == 0 ||
            strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
            strcmp(op, ">") == 0 || strcmp(op, "<") == 0) {
            advance();
            printf("    Comparison(%s)\n", tokens[current - 1].value);
            parse_term();
        }
    }
}

void parse_import() {
    if (match(TOKEN_KEYWORD, "import")) {
        if (match(TOKEN_STRING, NULL)) {
            printf("Import: %s\n", tokens[current - 1].value);
        } else {
            printf("Error: expected string after 'import'\n");
        }
    }
}

// Parse variable declaration
void parse_variable_declaration() {
    if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
         match(TOKEN_KEYWORD, "moment") ||
         match(TOKEN_KEYWORD, "flag")) {
        printf("VariableDecl: type=%s ", tokens[current - 1].value);
        if (match(TOKEN_IDENTIFIER, NULL)) {
            printf("name=%s", tokens[current - 1].value);
            if (match(TOKEN_SYMBOL, "=")) {
                printf(" = ");
                parse_expression();
            }
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after variable declaration\n");
            }
            printf("\n");
        } else {
            printf("Error: expected identifier\n");
        }
    }
}

// Parse tickout (output) statement
void parse_tickout() {
    if (match(TOKEN_KEYWORD, "tickout")) {
        printf("Tickout: ");
        parse_expression();
        if (!match(TOKEN_SYMBOL, ";")) {
            printf("Error: expected ';' after tickout\n");
        }
        printf("\n");
    }
}

// Parse tickin (input) statement
void parse_tickin() {
    if (match(TOKEN_KEYWORD, "tickin")) {
        if (match(TOKEN_IDENTIFIER, NULL)) {
            printf("Tickin: %s\n", tokens[current - 1].value);
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after tickin\n");
            }
        } else {
            printf("Error: expected identifier after tickin\n");
        }
    }
}

// Parse finish (return) statement
void parse_finish() {
    if (match(TOKEN_KEYWORD, "finish")) {
        printf("Finish: ");
        if (!check(TOKEN_SYMBOL, ";")) {
            parse_expression();
        }
        if (!match(TOKEN_SYMBOL, ";")) {
            printf("Error: expected ';' after finish\n");
        }
        printf("\n");
    }
}

void parse_when() {
    if (match(TOKEN_KEYWORD, "when")) {
        printf("When: condition(\n");
        if (match(TOKEN_SYMBOL, "(")) {
            parse_expression();
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after when condition\n");
            }
        }
        printf(") then:\n");
        if (match(TOKEN_SYMBOL, "{")) {
            parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after when block\n");
            }
        }
        // Check for otherwise
        if (check(TOKEN_KEYWORD, "otherwise")) {
            parse_otherwise();
        }
        printf("WhenEnd\n");
    }
}

void parse_otherwise() {
    if (match(TOKEN_KEYWORD, "otherwise")) {
        printf("Otherwise:\n");
        if (match(TOKEN_SYMBOL, "{")) {
            parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after otherwise block\n");
            }
        }
    }
}

// Parse repeat (while) loop
void parse_repeat() {
    if (match(TOKEN_KEYWORD, "repeat")) {
        printf("Repeat: condition(\n");
        if (match(TOKEN_SYMBOL, "(")) {
            parse_expression();
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after repeat condition\n");
            }
        }
        printf(") do:\n");
        if (match(TOKEN_SYMBOL, "{")) {
            parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after repeat block\n");
            }
        }
        printf("RepeatEnd\n");
    }
}

// Parse loop (for) loop
void parse_loop() {
    if (match(TOKEN_KEYWORD, "loop")) {
        printf("Loop: for(\n");
        if (match(TOKEN_SYMBOL, "(")) {
            // Parse initialization (type identifier = value;)
            if (!check(TOKEN_SYMBOL, ";")) {
                if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
                    match(TOKEN_KEYWORD, "moment") ||
                    match(TOKEN_KEYWORD, "flag")) {
                    printf("  InitType: %s ", tokens[current - 1].value);
                    if (match(TOKEN_IDENTIFIER, NULL)) {
                        printf("InitVar: %s", tokens[current - 1].value);
                        if (match(TOKEN_SYMBOL, "=")) {
                            printf(" = ");
                            parse_expression();
                        }
                    }
                }
            }
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after loop initialization\n");
            }
            // Parse condition
            if (!check(TOKEN_SYMBOL, ";")) {
                printf("  Condition: ");
                parse_expression();
            }
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' in loop condition\n");
            }
            // Parse increment (handle i++ or i = i + 1 style)
            if (!check(TOKEN_SYMBOL, ")")) {
                printf("  Increment: ");
                // Check for postfix increment (identifier++)
                if (peek()->type == TOKEN_IDENTIFIER && 
                    current + 1 < token_count &&
                    tokens[current + 1].type == TOKEN_SYMBOL &&
                    strcmp(tokens[current + 1].value, "+") == 0 &&
                    current + 2 < token_count &&
                    tokens[current + 2].type == TOKEN_SYMBOL &&
                    strcmp(tokens[current + 2].value, "+") == 0) {
                    advance(); // identifier
                    advance(); // +
                    advance(); // +
                    printf("PostfixIncrement(%s)\n", tokens[current - 3].value);
                } else {
                    parse_expression();
                }
            }
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after loop header\n");
            }
        }
        printf(") do:\n");
        if (match(TOKEN_SYMBOL, "{")) {
            parse_block();
            if (!match(TOKEN_SYMBOL, "}")) {
                printf("Error: expected '}' after loop block\n");
            }
        }
        printf("LoopEnd\n");
    }
}


void parse_block() {
    while (peek() && peek()->type != TOKEN_EOF && 
           !check(TOKEN_SYMBOL, "}")) {
        parse_statement();
    }
}


void parse_statement() {
    if (current >= token_count) return;
    

    if (peek()->type == TOKEN_COMMENT) {
        advance();
        return;
    }
    

    if (check(TOKEN_KEYWORD, "import")) {
        parse_import();
        return;
    }
    
    if (check(TOKEN_KEYWORD, "second") || check(TOKEN_KEYWORD, "minute") ||
         check(TOKEN_KEYWORD, "moment") ||
        check(TOKEN_KEYWORD, "flag")) {
        parse_variable_declaration();
        return;
    }
    
    if (check(TOKEN_KEYWORD, "tickout")) {
        parse_tickout();
        return;
    }
    
    if (check(TOKEN_KEYWORD, "tickin")) {
        parse_tickin();
        return;
    }
    
    if (check(TOKEN_KEYWORD, "finish")) {
        parse_finish();
        return;
    }
    
    if (check(TOKEN_KEYWORD, "when")) {
        parse_when();
        return;
    }
 
    if (check(TOKEN_KEYWORD, "repeat")) {
        parse_repeat();
        return;
    }
    
    if (check(TOKEN_KEYWORD, "loop")) {
        parse_loop();
        return;
    }
    
    
    if (peek()->type == TOKEN_IDENTIFIER) {
        Token* ident = peek();
        advance();
        if (check(TOKEN_SYMBOL, "=")) {
            printf("Assignment: %s = ", ident->value);
            advance(); 
            parse_expression();
            if (!match(TOKEN_SYMBOL, ";")) {
                printf("Error: expected ';' after assignment\n");
            }
            printf("\n");
            return;
        } else {
            current--; 
        }
    }
    
    parse_expression();
    if (match(TOKEN_SYMBOL, ";")) {
        printf("ExpressionStatement\n");
    }
}


void parse_startClock() {
    if (match(TOKEN_KEYWORD, "startClock")) {
        printf("StartClock: main(");
        if (match(TOKEN_SYMBOL, "(")) {
            if (!check(TOKEN_SYMBOL, ")")) {
                if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
                    match(TOKEN_KEYWORD, "moment") ||
                    match(TOKEN_KEYWORD, "flag")) {
                    printf(" %s", tokens[current - 1].value);
                    if (match(TOKEN_IDENTIFIER, NULL)) {
                        printf(" %s", tokens[current - 1].value);
                    }
                }
                while (match(TOKEN_SYMBOL, ",")) {
                    printf(",");
                    if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
                        match(TOKEN_KEYWORD, "moment") ||
                        match(TOKEN_KEYWORD, "flag")) {
                        printf(" %s", tokens[current - 1].value);
                        if (match(TOKEN_IDENTIFIER, NULL)) {
                            printf(" %s", tokens[current - 1].value);
                        }
                    }
                }
            }
            if (!match(TOKEN_SYMBOL, ")")) {
                printf("Error: expected ')' after startClock parameters\n");
            }
            printf(")\n");
            if (match(TOKEN_SYMBOL, "{")) {
                parse_block();
                if (!match(TOKEN_SYMBOL, "}")) {
                    printf("Error: expected '}' after startClock body\n");
                }
            } else {
                printf("Error: expected '{' after startClock()\n");
            }
        } else {
            printf("Error: expected '(' after startClock\n");
        }
        printf("StartClockEnd\n");
    }
}

void parse_function() {
    if (match(TOKEN_KEYWORD, "schedule")) {
        if (match(TOKEN_IDENTIFIER, NULL)) {
            printf("Function: %s(", tokens[current - 1].value);
            if (match(TOKEN_SYMBOL, "(")) {
              
                if (!check(TOKEN_SYMBOL, ")")) {
                    
                    if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
                         match(TOKEN_KEYWORD, "moment") ||
                        match(TOKEN_KEYWORD, "flag")) {
                        printf(" %s", tokens[current - 1].value);
                        if (match(TOKEN_IDENTIFIER, NULL)) {
                            printf(" %s", tokens[current - 1].value);
                        }
                    }
                    while (match(TOKEN_SYMBOL, ",")) {
                        printf(",");
                        if (match(TOKEN_KEYWORD, "second") || match(TOKEN_KEYWORD, "minute") ||
                             match(TOKEN_KEYWORD, "moment") ||
                            match(TOKEN_KEYWORD, "flag")) {
                            printf(" %s", tokens[current - 1].value);
                            if (match(TOKEN_IDENTIFIER, NULL)) {
                                printf(" %s", tokens[current - 1].value);
                            }
                        }
                    }
                }
                if (!match(TOKEN_SYMBOL, ")")) {
                    printf("Error: expected ')' after function parameters\n");
                }
            }
            printf(")\n");
            if (match(TOKEN_SYMBOL, "{")) {
                parse_block();
                if (!match(TOKEN_SYMBOL, "}")) {
                    printf("Error: expected '}' after function body\n");
                }
            }
            printf("FunctionEnd: %s\n", tokens[current - 2].value);
        }
    }
}


void parse_program() {
    current = 0;
    printf("=== Parsing 3AM Script ===\n\n");
    
    while (peek() && peek()->type != TOKEN_EOF) {
        if (peek()->type == TOKEN_COMMENT) {
            advance();
            continue;
        }
        
        if (check(TOKEN_KEYWORD, "timeline")) {
            advance();
            if (match(TOKEN_IDENTIFIER, NULL)) {
                printf("Timeline: %s\n", tokens[current - 1].value);
            }
            continue;
        }
        
        if (check(TOKEN_KEYWORD, "startClock")) {
            parse_startClock();
            continue;
        }
        
        if (check(TOKEN_KEYWORD, "schedule")) {
            parse_function();
            continue;
        }

        parse_statement();
    }
    
    printf("\n=== Parsing Complete ===\n");
}
