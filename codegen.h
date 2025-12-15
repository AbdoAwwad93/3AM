#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

// Generates C code from the given AST root.
// Returns 0 on success, non-zero on failure.
int generate_code(ASTNode* root, const char* output_filename);

#endif
