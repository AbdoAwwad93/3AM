#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"

#define MAX_TOKENS 1000
#define MAX_TOKEN_LENGTH 256

Token tokens[MAX_TOKENS];
int token_count = 0;
static int current_line = 1;

static int is_keyword(const char* word) {
    const char* keywords[] = {
        "startClock", "schedule", "tickout", "tickin", "when",
        "otherwise", "repeat", "loop", "finish", "timeline",
        "import", "second", "minute", "moment", "flag", "void",
        NULL
    };
    
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void add_token(TokenType type, const char* value) {
    if (token_count >= MAX_TOKENS) {
        fprintf(stderr, "Warning: Maximum token limit reached\n");
        return;
    }
    
    tokens[token_count].type = type;
    tokens[token_count].line = current_line;
    strncpy(tokens[token_count].value, value, MAX_TOKEN_LENGTH - 1);
    tokens[token_count].value[MAX_TOKEN_LENGTH - 1] = '\0';
    token_count++;
}

int scan_File(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file: %s\n", filename);
        return 0;
    }
    
    token_count = 0;
    current_line = 1;
    
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (isspace(ch)) {
            if (ch == '\n') current_line++;
            continue;
        }
        
        if (ch == '#') {
            char buffer[MAX_TOKEN_LENGTH] = {0};
            int i = 0;
            while ((ch = fgetc(file)) != '\n' && ch != EOF && i < MAX_TOKEN_LENGTH - 1) {
                buffer[i++] = ch;
            }
            if (ch == '\n') current_line++;
            buffer[i] = '\0';
            add_token(TOKEN_COMMENT, buffer);
        }
        else if (ch == '"') {
            char buffer[MAX_TOKEN_LENGTH] = {0};
            int i = 0;
            while ((ch = fgetc(file)) != '"' && ch != EOF && i < MAX_TOKEN_LENGTH - 1) {
                if (ch == '\n') current_line++;
                buffer[i++] = ch;
            }
            buffer[i] = '\0';
            add_token(TOKEN_STRING, buffer);
        }
        else if (isalpha(ch) || ch == '_') {
            char buffer[MAX_TOKEN_LENGTH] = {0};
            int i = 0;
            buffer[i++] = ch;
            while ((ch = fgetc(file)) != EOF && (isalnum(ch) || ch == '_') && i < MAX_TOKEN_LENGTH - 1) {
                buffer[i++] = ch;
            }
            ungetc(ch, file);
            buffer[i] = '\0';
            
            if (is_keyword(buffer)) {
                add_token(TOKEN_KEYWORD, buffer);
            } else {
                add_token(TOKEN_IDENTIFIER, buffer);
            }
        }
        else if (isdigit(ch)) {
            char buffer[64] = {0};
            int i = 0;
            buffer[i++] = ch;
            
            while (isdigit(ch = fgetc(file)) && i < 63) {
                buffer[i++] = ch;
            }
            
            if (ch == '.' && i < 63) {
                buffer[i++] = ch;
                while (isdigit(ch = fgetc(file)) && i < 63) {
                    buffer[i++] = ch;
                }
            }
            
            ungetc(ch, file);
            buffer[i] = '\0';
            add_token(TOKEN_NUMBER, buffer);
        }
        else {
            char symbol[3] = {ch, '\0', '\0'};
            
            if (ch == '=' || ch == '!' || ch == '<' || ch == '>') {
                char next = fgetc(file);
                if (next == '=') {
                    symbol[1] = next;
                } else {
                    ungetc(next, file);
                }
            } else if (ch == '+') {
                char next = fgetc(file);
                if (next == '+') {
                    symbol[1] = next;
                } else {
                    ungetc(next, file);
                }
            } else if (ch == '-') {
                char next = fgetc(file);
                if (next == '-') {
                    symbol[1] = next;
                } else {
                    ungetc(next, file);
                }
            }
            
            add_token(TOKEN_SYMBOL, symbol);
        }
    }
    
    add_token(TOKEN_EOF, "");
    fclose(file);
    return 1;
}

Token* get_tokens(void) {
    return tokens;
}

int get_token_count(void) {
    return token_count;
}
