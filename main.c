#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "ast.h"
#include "semantic.h"
#include "codegen.h"

extern int scan_File(const char* filename);
extern Token* get_tokens(void);
extern int get_token_count(void);
extern ASTNode* parse_program(void);

static void print_tokens(Token* tokens, int count) {
    printf("\n=== TOKENS ===\n");
    for (int i = 0; i < count; i++) {
        const char* type_name;
        switch (tokens[i].type) {
            case TOKEN_KEYWORD:    type_name = "KEYWORD"; break;
            case TOKEN_IDENTIFIER: type_name = "IDENTIFIER"; break;
            case TOKEN_NUMBER:     type_name = "NUMBER"; break;
            case TOKEN_STRING:     type_name = "STRING"; break;
            case TOKEN_SYMBOL:     type_name = "SYMBOL"; break;
            case TOKEN_COMMENT:    type_name = "COMMENT"; break;
            case TOKEN_EOF:        type_name = "EOF"; break;
            default:               type_name = "UNKNOWN"; break;
        }
        printf("Token %d: %s '%s'\n", i, type_name, tokens[i].value);
    }
    printf("\n");
}

static void print_compilation_header(const char* filename) {
    printf("\n=== 3AM LANGUAGE COMPILER v1.0 ===\n");
    printf("Compiling: %s\n\n", filename);
}

static void print_phase_header(const char* phase_name) {
    printf("\n--- PHASE: %s ---\n\n", phase_name);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("3AM Language Compiler\n");
        printf("Usage: %s <input.3am> [output.txt]\n", argv[0]);
        return 1;
    }
    
    // Output file handling moved to code generation phase
    /* 
    FILE* output_file = NULL;
    if (argc >= 3 && argv[2] && argv[2][0] != '\0') {
        output_file = freopen(argv[2], "w", stdout);
        if (!output_file) {
            fprintf(stderr, "Error: Cannot open output file: %s\n", argv[2]);
            return 1;
        }
    }
    */
    FILE* output_file = NULL; // Keep variable for compatibility with existing cleanup code
    
    print_compilation_header(argv[1]);
    
    print_phase_header("LEXICAL ANALYSIS (Scanning)");
    if (!scan_File(argv[1])) {
        fprintf(stderr, "Error: Failed to scan file '%s'\n", argv[1]);
        if (output_file) fclose(output_file);
        return 1;
    }
    
    Token* tokens = get_tokens();
    int token_count = get_token_count();
    printf("Lexical analysis complete. Found %d tokens.\n", token_count);
    print_tokens(tokens, token_count);
    
    print_phase_header("SYNTAX ANALYSIS (Parsing)");
    ASTNode* root = parse_program();
    
    if (!root) {
        fprintf(stderr, "Error: Parsing failed - no AST generated\n");
        if (output_file) fclose(output_file);
        return 1;
    }
    
    printf("Parsing complete. AST generated successfully.\n\n");
    printf("=== ABSTRACT SYNTAX TREE ===\n");
    print_ast(root, 0);
    
    print_phase_header("SEMANTIC ANALYSIS");
    int semantic_result = semantic_check(root);
    
    printf("\n=== COMPILATION SUMMARY ===\n");
    if (semantic_result == 0) {
        printf("Status: SUCCESS\n");
        printf("All phases completed without errors.\n");
        
        if (output_file || (argc >= 3)) {
             const char* out_name = (argc >= 3) ? argv[2] : "output.c";
             printf("\nGenerating code to: %s\n", out_name);
             if (generate_code(root, out_name) == 0) {
                 printf("Code generation successful.\n");
             } else {
                 printf("Code generation failed.\n");
             }
        }
    } else {
        printf("Status: FAILED\n");
        printf("Semantic errors: %d\n", semantic_result);
    }
    printf("\n");
    
    free_ast(root);
    if (output_file) fclose(output_file);
    
    return (semantic_result == 0) ? 0 : 1;
}
