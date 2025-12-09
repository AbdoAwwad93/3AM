#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_FLAG,
    TYPE_VOID,
    TYPE_FUNCTION
} Type;

typedef enum {
    KIND_VARIABLE,
    KIND_FUNCTION
} SymbolKind;

typedef struct {
    char *name;
    SymbolKind kind;
    Type type;
    int scope_level;
    int line;
    Type *param_types;
    int param_count;
    Type return_type;
} Symbol;

typedef struct Scope {
    int level;
    Symbol **symbols;
    int count;
    int capacity;
    struct Scope *parent;
} Scope;

/* Initialization and Cleanup */
void sym_init(void);
void sym_free(void);

/* Scope Management */
void enter_scope(void);
void exit_scope(void);

/* Symbol Declaration */
int declare_variable_symbol(const char *name, Type t, int line);
int declare_function_symbol(const char *name, Type ret_type, Type *param_types, int param_count, int line);

/* Symbol Lookup */
Symbol *lookup_local(const char *name);
Symbol *lookup_symbol(const char *name);

/* Type Conversion */
Type make_type_from_string(const char *s);
const char *type_to_string(Type t);
int types_compatible(Type target, Type source);

#endif
