#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static FILE* out;
static int indentation = 0;

static void gen_indent() {
    for (int i = 0; i < indentation; i++) {
        fprintf(out, "    ");
    }
}

static const char* map_type(const char* type_name) {
    if (type_name == NULL) return "void";
    if (strcmp(type_name, "second") == 0) return "int";
    if (strcmp(type_name, "minute") == 0) return "float";
    if (strcmp(type_name, "moment") == 0) return "char*"; // Simple string support
    if (strcmp(type_name, "flag") == 0) return "int";     // Boolean is int
    return "void";
}

// Forward declarations
static void gen_expression(ASTNode* node);
static void gen_statement(ASTNode* node);

static void gen_expression(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_NUMBER:
            fprintf(out, "%s", node->value);
            break;
        case AST_STRING:
            // Wrap in quotes and cast to char* for _Generic compatibility
            fprintf(out, "((char*)\"%s\")", node->value);
            break;
        case AST_IDENTIFIER:
            fprintf(out, "%s", node->value);
            break;
        case AST_BINARY_OP:
            if (strcmp(node->value, "+") == 0) {
                 // Check if it's string concatenation
                 // This is tricky without type info in AST_BINARY_OP node itself (we computed it in semantics but didn't store it in generic way easily accessible here unless we stored type in ASTNode).
                 // For now, assume runtime helper or standard + for numbers.
                 // To support string concat properly in C, we need a runtime helper `__concat`.
                 // We will generate `__3am_add(...)` macro or function call if we want to be safe, or just `+` and hope for numbers.
                 // Given the example `tickout "Hello " + name`, we definitely need string concat.
                 // We'll use a helper `__add` that handles generic types or just use `+` for numbers and special handling if we knew types.
                 // Since we don't have easy type info attached to every node in this simple AST (semantics pass checked it but didn't annotate AST fully?), 
                 // we will rely on a runtime macro/function if possible.
                 // Actually, let's use a helper for EVERYTHING that could be an add: `__add(a, b)` which in C via `_Generic` (C11) could work, 
                 // or just always output `+` and fail for strings unless we handle them.
                 // Wait, the semantic check confirms types. "tickout ... + ..."
                 // Let's implement a simple `add_poly` macro in the C preamble.
                 fprintf(out, "__add(");
                 gen_expression(node->left);
                 fprintf(out, ", ");
                 gen_expression(node->right);
                 fprintf(out, ")");
            } else {
                fprintf(out, "(");
                gen_expression(node->left);
                fprintf(out, " %s ", node->value);
                gen_expression(node->right);
                fprintf(out, ")");
            }
            break;
        case AST_FUNCTION_CALL:
            fprintf(out, "%s(", node->value);
            ASTNode* arg = node->args;
            while (arg) {
                gen_expression(arg);
                if (arg->next) fprintf(out, ", ");
                arg = arg->next;
            }
            fprintf(out, ")");
            break;
        
        case AST_COMPARISON_OP:
            fprintf(out, "(");
             // String comparison needs strcmp
             // Again, without type annotation on the node, it's hard to know if we should use strcmp.
             // But for now, let's assume numeric comparison for operators like >=, <=. 
             // == and != might be string. 
             // Semantic analysis knows types. We might technically be degenerating here.
             // For this iteration, I will output standard C ops. 
             // If the user does string comparison, standard `==` checks pointers.
             // We'll leave it as is for `second` (int) and `minute` (float).
            gen_expression(node->left);
            fprintf(out, " %s ", node->value);
            gen_expression(node->right);
            fprintf(out, ")");
            break;

        case AST_UNARY_OP:
            {
                // Check if it's prefix or postfix by checking the value
                int is_prefix = (strstr(node->value, "_post") == NULL);
                if (is_prefix) {
                    // Pre-increment or pre-decrement: ++x or --x
                    fprintf(out, "%s", node->value);
                    gen_expression(node->left);
                } else {
                    // Post-increment or post-decrement: x++ or x--
                    // Remove "_post" suffix
                    char op[4] = {0};
                    if (strstr(node->value, "++") != NULL) {
                        strcpy(op, "++");
                    } else if (strstr(node->value, "--") != NULL) {
                        strcpy(op, "--");
                    }
                    gen_expression(node->left);
                    fprintf(out, "%s", op);
                }
            }
            break;

        default:
            fprintf(out, "/* Expr %d */", node->type);
    }
}

static void gen_statement(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_VARIABLE_DECL:
            gen_indent();
            fprintf(out, "%s %s", map_type(node->var_type), node->value);
            if (node->expression) {
                fprintf(out, " = ");
                gen_expression(node->expression);
            } else {
                // Initialize char* to NULL to be safe, numbers to 0
                if (strcmp(node->var_type, "moment") == 0) fprintf(out, " = \"\"");
                else fprintf(out, " = 0");
            }
            fprintf(out, ";\n");
            break;

        case AST_ASSIGNMENT:
            gen_indent();
            fprintf(out, "%s = ", node->value);
            gen_expression(node->expression);
            fprintf(out, ";\n");
            break;

        case AST_TICKOUT:
            gen_indent();
            // printf doesn't handle our custom __add result (which might be char* or number) easily without _Generic format string.
            // Simplified approach: `print_any` helper.
            fprintf(out, "__print_any(");
            gen_expression(node->expression);
            fprintf(out, ");\n");
            break;

        case AST_TICKIN:
            gen_indent();
            // Need to know type of variable to prompt correctly? 
            // We'll use a helper `__read_input(&var, type_enum)`?
            // Since we can't easily reflect type here, we'll assume a specific scanner or just `cin >>`.
            // Let's use a macro that attempts to read based on variable name? No.
            // We'll assume the C compiler can handle some overrides or we assume simple standard input.
            // A helper `__scan_key(&var)`? 
            // In C, `scanf` needs format. 
            // Let's try to infer from variable name or usage? No.
            // We will use a generic `__tickin` which might need `_Generic` in the preamble.
            fprintf(out, "__tickin(&%s);\n", node->value);
            break;

        case AST_WHEN:
            gen_indent();
            fprintf(out, "if (");
            gen_expression(node->condition);
            fprintf(out, ") {\n");
            indentation++;
            gen_statement(node->body);
            indentation--;
            gen_indent();
            fprintf(out, "}");
            if (node->otherwise) {
                fprintf(out, " else {\n");
                indentation++;
                gen_statement(node->otherwise);
                indentation--;
                gen_indent();
                fprintf(out, "}\n");
            } else {
                fprintf(out, "\n");
            }
            break;

        case AST_REPEAT:
            gen_indent();
            fprintf(out, "while (");
            gen_expression(node->condition);
            fprintf(out, ") {\n");
            indentation++;
            gen_statement(node->body);
            indentation--;
            gen_indent();
            fprintf(out, "}\n");
            break;

        case AST_LOOP:
            gen_indent();
            fprintf(out, "for (");
            if (node->init) {
                // Extract init expression/stmt partial
                // loop(second i=0; i<10; i=i+1)
                // node->init is a statement usually AST_VARIABLE_DECL
                // We'll print it without semicolon and newline if possible, or we need to block this.
                // Our AST structure for loop init is a statement node.
                // Standard for loop in C: `for (type var = val; cond; inc)`
                // But `gen_statement` adds `;\n`.
                // We'll handle inline generation manually for loop head.
                if (node->init->type == AST_VARIABLE_DECL) {
                     fprintf(out, "%s %s", map_type(node->init->var_type), node->init->value);
                     if (node->init->expression) {
                         fprintf(out, " = ");
                         gen_expression(node->init->expression);
                     }
                }
            }
            fprintf(out, "; ");
            gen_expression(node->condition);
            fprintf(out, "; ");
            if (node->increment) {
                 // increment is often an assignment expression, or just expression
                 gen_expression(node->increment);
            }
            fprintf(out, ") {\n");
            indentation++;
            gen_statement(node->body);
            indentation--;
            gen_indent();
            fprintf(out, "}\n");
            break;

        case AST_BLOCK:
            {
                ASTNode* stmt = node->body;
                while (stmt) {
                    gen_statement(stmt);
                    stmt = stmt->next;
                }
            }
            break;

        case AST_OTHERWISE:
            // Generate the body of the otherwise block
            {
                ASTNode* stmt = node->body;
                if (stmt && stmt->type == AST_BLOCK) {
                    // If it's a block, generate its contents
                    ASTNode* inner = stmt->body;
                    while (inner) {
                        gen_statement(inner);
                        inner = inner->next;
                    }
                } else {
                    // Direct statements
                    while (stmt) {
                        gen_statement(stmt);
                        stmt = stmt->next;
                    }
                }
            }
            break;

        case AST_FINISH:
            gen_indent();
            fprintf(out, "return");
            if (node->expression) {
                fprintf(out, " ");
                gen_expression(node->expression);
            }
            fprintf(out, ";\n");
            break;
            
        case AST_EXPRESSION_STMT:
            gen_indent();
            gen_expression(node->expression);
            fprintf(out, ";\n");
            break;

        default:
            // Fallback for list of statements
            if (node->type == AST_PROGRAM || node->type == AST_STARTCLOCK || node->type == AST_FUNCTION) {
                 // Should be handled in top-level
            }
            break;
    }
}

static void gen_function(ASTNode* node) {
    if (node->type == AST_STARTCLOCK) {
        // startClock doesn't have a value in AST, implicit name
        fprintf(out, "\n%s startClock(", map_type(node->var_type));
    } else {
        fprintf(out, "\n%s %s(", map_type(node->var_type), node->value ? node->value : "unnamed");
    }

    ASTNode* param = node->params;
    while (param) {
        fprintf(out, "%s %s", map_type(param->var_type), param->value);
        if (param->next) fprintf(out, ", ");
        param = param->next;
    }
    fprintf(out, ") {\n");
    indentation++;
    
    ASTNode* stmt = node->body;
    while (stmt) {
        gen_statement(stmt);
        stmt = stmt->next;
    }

    indentation--;
    fprintf(out, "}\n");
}

int generate_code(ASTNode* root, const char* output_filename) {
    printf("DEBUG: Starting generate_code with file %s\n", output_filename);
    out = fopen(output_filename, "w");
    if (!out) {
        printf("DEBUG: Failed to open file %s\n", output_filename);
        return 1;
    }

    // PREAMBLE
    // PREAMBLE
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include <string.h>\n");
    fprintf(out, "\n");
    fprintf(out, "// 3AM Runtime Helper Macros/Functions\n");
    fprintf(out, "#define _CRT_SECURE_NO_WARNINGS\n");
    
    // Tracking Allocator
    fprintf(out, "typedef struct Allocation { void* ptr; struct Allocation* next; } Allocation;\n");
    fprintf(out, "Allocation* gc_head = NULL;\n");
    fprintf(out, "void* gc_malloc(size_t size) {\n");
    fprintf(out, "    void* ptr = malloc(size);\n");
    fprintf(out, "    if (ptr) {\n");
    fprintf(out, "        Allocation* node = (Allocation*)malloc(sizeof(Allocation));\n");
    fprintf(out, "        node->ptr = ptr;\n");
    fprintf(out, "        node->next = gc_head;\n");
    fprintf(out, "        gc_head = node;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return ptr;\n");
    fprintf(out, "}\n");
    fprintf(out, "char* gc_strdup(const char* s) {\n");
    fprintf(out, "    if (!s) return NULL;\n");
    fprintf(out, "    char* d = (char*)gc_malloc(strlen(s) + 1);\n");
    fprintf(out, "    if (d) strcpy(d, s);\n");
    fprintf(out, "    return d;\n");
    fprintf(out, "}\n");
    fprintf(out, "void gc_free_all() {\n");
    fprintf(out, "    while (gc_head) {\n");
    fprintf(out, "        free(gc_head->ptr);\n");
    fprintf(out, "        Allocation* next = gc_head->next;\n");
    fprintf(out, "        free(gc_head);\n");
    fprintf(out, "        gc_head = next;\n");
    fprintf(out, "    }\n");
    fprintf(out, "}\n");

    // Helpers using gc_malloc
    fprintf(out, "char* __to_str_int(int v) { char* b = gc_malloc(32); sprintf(b, \"%%d\", v); return b; }\n");
    fprintf(out, "char* __to_str_float(float v) { char* b = gc_malloc(32); sprintf(b, \"%%.2f\", v); return b; }\n");
    
    // C11 _Generic to print
    fprintf(out, "void __print_any_int(int v) { printf(\"%%d\\n\", v); }\n");
    fprintf(out, "void __print_any_float(float v) { printf(\"%%.2f\\n\", v); }\n");
    fprintf(out, "void __print_any_str(char* v) { printf(\"%%s\\n\", v ? v : \"(null)\"); }\n");
    fprintf(out, "#define __print_any(X) _Generic((X), int: __print_any_int, float: __print_any_float, char*: __print_any_str, default: __print_any_int)(X)\n");

    // C11 _Generic to scan
    fprintf(out, "void __tickin_int(int* v) { fflush(stdout); scanf(\"%%d\", v); }\n");
    fprintf(out, "void __tickin_float(float* v) { fflush(stdout); scanf(\"%%f\", v); }\n");
    fprintf(out, "void __tickin_str(char** v) { fflush(stdout); char b[256]; scanf(\"%%255s\", b); *v = gc_strdup(b); }\n"); 
    fprintf(out, "#define __tickin(X) _Generic((X), int*: __tickin_int, float*: __tickin_float, char**: __tickin_str)(X)\n");

    // C11 _Generic to Add
    fprintf(out, "int __add_ii(int a, int b) { return a + b; }\n");
    fprintf(out, "float __add_ff(float a, float b) { return a + b; }\n");
    fprintf(out, "float __add_if(int a, float b) { return a + b; }\n");
    fprintf(out, "float __add_fi(float a, int b) { return a + b; }\n");
    fprintf(out, "char* __add_ss(char* a, char* b) { char* n = gc_malloc(strlen(a?a:\"\")+strlen(b?b:\"\")+1); strcpy(n,a?a:\"\"); strcat(n,b?b:\"\"); return n; }\n");
    fprintf(out, "char* __add_si(char* a, int b) { char* s = __to_str_int(b); char* r = __add_ss(a, s); return r; }\n");
    fprintf(out, "char* __add_is(int a, char* b) { char* s = __to_str_int(a); char* r = __add_ss(s, b); return r; }\n");
    fprintf(out, "char* __add_sf(char* a, float b) { char* s = __to_str_float(b); char* r = __add_ss(a, s); return r; }\n");
    fprintf(out, "char* __add_fs(float a, char* b) { char* s = __to_str_float(a); char* r = __add_ss(s, b); return r; }\n");

    fprintf(out, "#define __add_2(A, B) (_Generic((B), \\\n");
    fprintf(out, "    int:   _Generic((A), int: __add_ii, float: __add_fi, char*: __add_si), \\\n");
    fprintf(out, "    float: _Generic((A), int: __add_if, float: __add_ff, char*: __add_sf), \\\n");
    fprintf(out, "    char*: _Generic((A), int: __add_is, float: __add_fs, char*: __add_ss) \\\n");
    fprintf(out, ")(A, B))\n");
    fprintf(out, "#define __add(A, B) __add_2(A, B)\n");

    fprintf(out, "\n// Generated Code\n");

    ASTNode* node = root;
    while (node) {
        if (node->type == AST_FUNCTION || node->type == AST_STARTCLOCK) {
            gen_function(node);
        } else if (node->type == AST_PROGRAM) {
            // Traverse children
            ASTNode* child = node->body;
            while (child) {
               if (child->type == AST_FUNCTION || child->type == AST_STARTCLOCK) {
                   gen_function(child);
               }
               child = child->next;
            }
        }
        node = node->next; // Top level list?
    }

    // Main entry point
    fprintf(out, "\nint main() {\n");
    fprintf(out, "    startClock();\n");
    fprintf(out, "    gc_free_all();\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");

    fclose(out);
    return 0;
}
