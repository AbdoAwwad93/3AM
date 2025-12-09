#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

int semantic_check(ASTNode *program_root);

extern int strict_finish_in_functions;


#endif
