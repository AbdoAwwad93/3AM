#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAP 8

static Scope *current_scope = NULL;
static int current_level = 0;

void sym_init(void) {
    current_scope = NULL;
    current_level = 0;
    enter_scope();
}

void sym_free(void) {
    while (current_scope) {
        exit_scope();
    }
}

static Symbol* make_symbol(const char *name, SymbolKind kind, Type t, int line) {
    Symbol *s = (Symbol*)malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->kind = kind;
    s->type = t;
    s->scope_level = current_level;
    s->line = line;
    s->param_types = NULL;
    s->param_count = 0;
    s->return_type = TYPE_VOID;
    return s;
}

void enter_scope(void) {
    Scope *sc = (Scope*)malloc(sizeof(Scope));
    sc->level = current_level;
    sc->count = 0;
    sc->capacity = INITIAL_CAP;
    sc->symbols = (Symbol**)malloc(sizeof(Symbol*) * sc->capacity);
    sc->parent = current_scope;
    current_scope = sc;
    current_level++;
}

void exit_scope(void) {
    if (!current_scope) return;
    
    for (int i = 0; i < current_scope->count; ++i) {
        Symbol *s = current_scope->symbols[i];
        if (s->param_types) free(s->param_types);
        if (s->name) free(s->name);
        free(s);
    }
    free(current_scope->symbols);
    
    Scope *parent = current_scope->parent;
    free(current_scope);
    current_scope = parent;
    current_level--;
    if (current_level < 0) current_level = 0;
}

int declare_variable_symbol(const char *name, Type t, int line) {
    if (!current_scope) return -1;
    
    for (int i = 0; i < current_scope->count; ++i) {
        if (strcmp(current_scope->symbols[i]->name, name) == 0) {
            return -1;
        }
    }
    
    if (current_scope->count >= current_scope->capacity) {
        current_scope->capacity *= 2;
        current_scope->symbols = (Symbol**)realloc(
            current_scope->symbols, 
            sizeof(Symbol*) * current_scope->capacity
        );
    }
    
    Symbol *s = make_symbol(name, KIND_VARIABLE, t, line);
    current_scope->symbols[current_scope->count++] = s;
    return 0;
}

int declare_function_symbol(const char *name, Type ret_type, Type *param_types, int param_count, int line) {
    if (!current_scope) return -1;
    
    for (int i = 0; i < current_scope->count; ++i) {
        if (strcmp(current_scope->symbols[i]->name, name) == 0) {
            return -1;
        }
    }
    
    if (current_scope->count >= current_scope->capacity) {
        current_scope->capacity *= 2;
        current_scope->symbols = (Symbol**)realloc(
            current_scope->symbols, 
            sizeof(Symbol*) * current_scope->capacity
        );
    }
    
    Symbol *s = make_symbol(name, KIND_FUNCTION, TYPE_FUNCTION, line);
    s->return_type = ret_type;  
    
    if (param_count > 0 && param_types) {
        s->param_types = (Type*)malloc(sizeof(Type) * param_count);
        for (int i = 0; i < param_count; ++i) {
            s->param_types[i] = param_types[i];
        }
    } else {
        s->param_types = NULL;
    }
    s->param_count = param_count;
    
    current_scope->symbols[current_scope->count++] = s;
    return 0;
}

Symbol *lookup_local(const char *name) {
    if (!current_scope) return NULL;
    for (int i = 0; i < current_scope->count; ++i) {
        if (strcmp(current_scope->symbols[i]->name, name) == 0) {
            return current_scope->symbols[i];
        }
    }
    return NULL;
}

Symbol *lookup_symbol(const char *name) {
    Scope *sc = current_scope;
    while (sc) {
        for (int i = 0; i < sc->count; ++i) {
            if (strcmp(sc->symbols[i]->name, name) == 0) {
                return sc->symbols[i];
            }
        }
        sc = sc->parent;
    }
    return NULL;
}

Type make_type_from_string(const char *s) {
    if (!s) return TYPE_UNKNOWN;
    if (strcmp(s, "second") == 0) return TYPE_INTEGER;
    if (strcmp(s, "minute") == 0) return TYPE_FLOAT;
    if (strcmp(s, "moment") == 0) return TYPE_STRING;
    if (strcmp(s, "flag") == 0) return TYPE_FLAG;
    if (strcmp(s, "void") == 0) return TYPE_VOID;
    return TYPE_UNKNOWN;
}

const char *type_to_string(Type t) {
    switch (t) {
        case TYPE_INTEGER:  return "integer (second)";
        case TYPE_FLOAT:    return "float (minute)";
        case TYPE_FLAG:     return "flag";
        case TYPE_STRING:   return "string (moment)";
        case TYPE_FUNCTION: return "function";
        case TYPE_VOID:     return "void";
        default:            return "unknown";
    }
}

int types_compatible(Type target, Type source) {
    if (target == source) return 1;
    if (target == TYPE_UNKNOWN || source == TYPE_UNKNOWN) return 1;
    if (target == TYPE_FLOAT && source == TYPE_INTEGER) return 1;
    if (target == TYPE_INTEGER && source == TYPE_FLOAT) return 0;
    return 0;
}
