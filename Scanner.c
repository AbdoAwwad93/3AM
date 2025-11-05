#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"

void parse_program();

int is_keyword(const char* word) {
    const char* keywords[] = {
    "startClock", "schedule", "tickout", "tickin", "when",
    "otherwise", "repeat", "loop", "finish", "timeline", "import",
    "second", "minute", "moment", "flag",

    NULL
};
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }
    return 0;
}
Token tokens[1000];
int token_count = 0;

void add_token(TokenType type, const char* value) {
    if (token_count >= 1000) return;
    tokens[token_count].type = type;
    strncpy(tokens[token_count].value, value, 255);
    tokens[token_count].value[255] = '\0';
    token_count++;
}

int scan_File(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open file\n");
        return 0;
    }

    token_count = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (isspace(ch)) continue;
    
        if (ch == '#') {
            char buffer[256] = {0};
            int i = 0;
            while ((ch = fgetc(file)) != '\n' && ch != EOF && i < 255)
                buffer[i++] = ch;
            buffer[i] = '\0';
            add_token(TOKEN_COMMENT, buffer);
        }

        else if (ch == '"') {
            char buffer[256] = {0};
            int i = 0;
            while ((ch = fgetc(file)) != '"' && ch != EOF && i < 255) {
                buffer[i++] = ch;
            }
            buffer[i] = '\0';
            add_token(TOKEN_STRING, buffer);
        }
        else if (isalpha(ch) || ch == '_') {
            char buffer[256] = {0};
            int i = 0;
            buffer[i++] = ch;
            while ((ch = fgetc(file)) != EOF && (isalnum(ch) || ch == '_') && i < 255) {
                buffer[i++] = ch;
            }
            ungetc(ch, file); // put back last char
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
                if (next == '=' && ch != '=') {
                    symbol[1] = next;
                    symbol[2] = '\0';
                } else {
                    ungetc(next, file);
                }
            }
            add_token(TOKEN_SYMBOL, symbol);
        }
    }
    
    // Add EOF token
    add_token(TOKEN_EOF, "");
    fclose(file);
    return 1;
}
Token* get_tokens() {
    return tokens;
}

int get_token_count() {
    return token_count;
}

void parse_program();

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    if (scan_File(argv[1])) {
        parse_program();
    }
    return 0;
}