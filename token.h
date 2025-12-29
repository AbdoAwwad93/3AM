#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_SYMBOL,
    TOKEN_COMMENT
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
    int line;
} Token;

#endif

