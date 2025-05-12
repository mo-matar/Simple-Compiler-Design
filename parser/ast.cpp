//ast.cpp
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "FileDescriptor.h"

// Type name strings for printing
static const char* type_names[] = {
    "none", "integer", "float", "boolean", "string"
};

/* Internal routines: */
static void nl_indent(FILE *fp, int indent);
static void p_a_n(FILE *fp, AST *node, int indent);
static void print_ast_list(FILE *fp, ast_list *list, const char *separator, int indent);
static void print_ste_list(FILE *fp, ste_list *list, const char *prefix, const char *separator, int indent);

// Error handling functions
static void fatal_error(const char* message) {
    fprintf(stderr, "FATAL ERROR: %s\n", message);
    exit(1);
}

// Helper function to get symbol name
static const char* ste_name(symbol_table_entry *entry) {
    return entry ? entry->Name : "unknown";
}

// Helper function to get variable type
static j_type ste_var_type(symbol_table_entry *entry) {
    return entry ? entry->VarType : type_none;
}

// Helper function to get constant value
static int ste_const_value(symbol_table_entry *entry) {
    return entry ? entry->ConstValue : 0;
}

// Create a new list cell with an AST node
ast_list *cons_ast(AST *head, ast_list *tail) {
    ast_list *cell = (ast_list *)malloc(sizeof(ast_list));
    if (cell == NULL) {
        fatal_error("Out of memory in cons_ast");
    }
    cell->head = head;
    cell->tail = tail;
    return cell;
}

// Create a new list cell with a symbol table entry
ste_list *cons_ste(symbol_table_entry *head, ste_list *tail) {
    ste_list *cell = (ste_list *)malloc(sizeof(ste_list));
    if (cell == NULL) {
        fatal_error("Out of memory in cons_ste");
    }
    cell->head = head;
    cell->tail = tail;
    return cell;
}

// Create an AST node with variable arguments
AST *make_ast_node(AST_type type, ...) {
    AST *node = (AST *)malloc(sizeof(AST));
    if (node == NULL) {
        fatal_error("Out of memory in make_ast_node");
    }
    
    va_list args;
    va_start(args, type);
    
    node->type = type;
    
    switch (type) {        case ast_var_decl:
            node->f.a_var_decl.name = va_arg(args, symbol_table_entry *);
            // Fix: j_type is promoted to int when passed through varargs
            node->f.a_var_decl.type = (j_type)va_arg(args, int);
            break;

        case ast_const_decl:
            node->f.a_const_decl.name = va_arg(args, symbol_table_entry *);
            node->f.a_const_decl.value = va_arg(args, int);
            break;        case ast_routine_decl:
            node->f.a_routine_decl.name = va_arg(args, symbol_table_entry *);
            node->f.a_routine_decl.formals = va_arg(args, ste_list *);
            // Fix: j_type is promoted to int when passed through varargs
            node->f.a_routine_decl.result_type = (j_type)va_arg(args, int);
            node->f.a_routine_decl.body = va_arg(args, AST *);
            break;

        case ast_assign:
            node->f.a_assign.lhs = va_arg(args, symbol_table_entry *);
            node->f.a_assign.rhs = va_arg(args, AST *);
            break;

        case ast_if:
            node->f.a_if.predicate = va_arg(args, AST *);
            node->f.a_if.conseq = va_arg(args, AST *);
            node->f.a_if.altern = va_arg(args, AST *);
            break;

        case ast_while:
            node->f.a_while.predicate = va_arg(args, AST *);
            node->f.a_while.body = va_arg(args, AST *);
            break;

        case ast_for:
            node->f.a_for.var = va_arg(args, symbol_table_entry *);
            node->f.a_for.lower_bound = va_arg(args, AST *);
            node->f.a_for.upper_bound = va_arg(args, AST *);
            node->f.a_for.body = va_arg(args, AST *);
            break;

        case ast_read:
            node->f.a_read.var = va_arg(args, symbol_table_entry *);
            break;

        case ast_write:
            node->f.a_write.var = va_arg(args, symbol_table_entry *);
            break;

        case ast_call:
            node->f.a_call.callee = va_arg(args, symbol_table_entry *);
            node->f.a_call.arg_list = va_arg(args, ast_list *);
            break;

        case ast_block:
            node->f.a_block.vars = va_arg(args, ste_list *);
            node->f.a_block.stmts = va_arg(args, ast_list *);
            break;

        case ast_return:
            node->f.a_return.expr = va_arg(args, AST *);
            break;

        case ast_var:
            node->f.a_var.var = va_arg(args, symbol_table_entry *);
            break;

        case ast_integer:
            node->f.a_integer.value = va_arg(args, int);
            break;

        case ast_float:
            node->f.a_float.value = (float)va_arg(args, double); // Float is promoted to double in varargs
            break;

        case ast_string:
            node->f.a_string.string = va_arg(args, char *);
            break;

        case ast_boolean:
            node->f.a_boolean.value = va_arg(args, int);
            break;

        case ast_times:
        case ast_divide:
        case ast_plus:
        case ast_minus:
        case ast_eq:
        case ast_neq:
        case ast_lt:
        case ast_le:
        case ast_gt:
        case ast_ge:
        case ast_and:
        case ast_or:
        case ast_cand:
        case ast_cor:
            node->f.a_binary_op.larg = va_arg(args, AST *);
            node->f.a_binary_op.rarg = va_arg(args, AST *);
            break;

        case ast_not:
        case ast_uminus:
            node->f.a_unary_op.arg = va_arg(args, AST *);
            break;

        case ast_itof:
            node->f.a_itof.arg = va_arg(args, AST *);
            break;

        case ast_eof:
            // No arguments needed
            break;

        case ast_program:
            node->f.a_program.statements = va_arg(args, ast_list *);
            break;

        default:
            fatal_error("Unknown AST node type in make_ast_node");
    }
    
    va_end(args);
    return node;
}

// Evaluate a constant expression
int eval_ast_expr(FileDescriptor *fd, AST *node) {
    if (node == NULL) {
        fatal_error("NULL AST in eval_ast_expr");
    }
    
    switch (node->type) {
        case ast_var:
            if (node->f.a_var.var->IsConstant) {
                return node->f.a_var.var->ConstValue;
            } else {
                fd->ReportError("Cannot use variables in constant expressions");
                return 0;
            }
            
        case ast_integer:
            return node->f.a_integer.value;
            
        case ast_string:
            fd->ReportError("Cannot use strings in constant expressions");
            return 0;
            
        case ast_boolean:
            return node->f.a_boolean.value;
            
        case ast_times:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) * 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_divide:
            {
                int divisor = eval_ast_expr(fd, node->f.a_binary_op.rarg);
                if (divisor == 0) {
                    fd->ReportError("Division by zero in constant expression");
                    return 0;
                }
                return eval_ast_expr(fd, node->f.a_binary_op.larg) / divisor;
            }
            
        case ast_plus:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) + 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_minus:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) - 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_eq:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) == 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_neq:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) != 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_lt:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) < 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_le:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) <= 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_gt:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) > 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_ge:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) >= 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_and:
        case ast_cand:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) && 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_or:
        case ast_cor:
            return eval_ast_expr(fd, node->f.a_binary_op.larg) || 
                   eval_ast_expr(fd, node->f.a_binary_op.rarg);
                   
        case ast_not:
            return !eval_ast_expr(fd, node->f.a_unary_op.arg);
            
        case ast_uminus:
            return -eval_ast_expr(fd, node->f.a_unary_op.arg);
            
        default:
            fd->ReportError("Unknown AST node type in eval_ast_expr");
            return 0;
    }
}

// Print an AST node
void print_ast_node(FILE *fp, AST *node) {
    p_a_n(fp, node, 0);
}

// Internal printing function with indentation
static void p_a_n(FILE *fp, AST *node, int indent) {
    if (node == NULL) return;
    
    switch (node->type) {
        case ast_var_decl:
            fprintf(fp, "var %s: %s;", ste_name(node->f.a_var_decl.name),
                   type_names[node->f.a_var_decl.type]);
            nl_indent(fp, indent);
            break;
            
        case ast_const_decl:
            fprintf(fp, "constant %s = %d;", ste_name(node->f.a_const_decl.name),
                   node->f.a_const_decl.value);
            nl_indent(fp, indent);
            break;
            
        case ast_routine_decl:
            if (node->f.a_routine_decl.result_type == type_none)
                fprintf(fp, "procedure %s (", ste_name(node->f.a_routine_decl.name));
            else
                fprintf(fp, "function %s (", ste_name(node->f.a_routine_decl.name));
                
            print_ste_list(fp, node->f.a_routine_decl.formals, "", ", ", -1);
            
            if (node->f.a_routine_decl.result_type == type_none) {
                fprintf(fp, ")");
                nl_indent(fp, indent + 2);
            } else {
                fprintf(fp, ") : %s", type_names[node->f.a_routine_decl.result_type]);
                nl_indent(fp, indent + 2);
            }
            
            p_a_n(fp, node->f.a_routine_decl.body, indent + 2);
            fprintf(fp, ";");
            nl_indent(fp, indent);
            break;
            
        case ast_assign:
            fprintf(fp, "%s := ", ste_name(node->f.a_assign.lhs));
            p_a_n(fp, node->f.a_assign.rhs, indent);
            break;
            
        case ast_if:
            fprintf(fp, "if ");
            p_a_n(fp, node->f.a_if.predicate, indent);
            fprintf(fp, " then");
            nl_indent(fp, indent + 2);
            p_a_n(fp, node->f.a_if.conseq, indent + 2);
            
            if (node->f.a_if.altern != NULL) {
                nl_indent(fp, indent);
                fprintf(fp, "else");
                nl_indent(fp, indent + 2);
                p_a_n(fp, node->f.a_if.altern, indent + 2);
            }
            break;
            
        case ast_while:
            fprintf(fp, "while ");
            p_a_n(fp, node->f.a_while.predicate, indent);
            fprintf(fp, " do");
            nl_indent(fp, indent + 2);
            p_a_n(fp, node->f.a_while.body, indent + 2);
            nl_indent(fp, indent);
            fprintf(fp, "od");
            break;
            
        case ast_for:
            fprintf(fp, "for %s := ", ste_name(node->f.a_for.var));
            p_a_n(fp, node->f.a_for.lower_bound, indent);
            fprintf(fp, " to ");
            p_a_n(fp, node->f.a_for.upper_bound, indent);
            fprintf(fp, " do");
            nl_indent(fp, indent + 2);
            p_a_n(fp, node->f.a_for.body, indent + 2);
            nl_indent(fp, indent);
            fprintf(fp, "od");
            break;
            
        case ast_read:
            fprintf(fp, "read(%s)", ste_name(node->f.a_read.var));
            break;
            
        case ast_write:
            fprintf(fp, "write(%s)", ste_name(node->f.a_write.var));
            break;
            
        case ast_call:
            fprintf(fp, "%s(", ste_name(node->f.a_call.callee));
            print_ast_list(fp, node->f.a_call.arg_list, ", ", -1);
            fprintf(fp, ")");
            break;
            
        case ast_block:
            fprintf(fp, "begin");
            nl_indent(fp, indent + 2);
            print_ste_list(fp, node->f.a_block.vars, "var ", "", indent + 2);
            print_ast_list(fp, node->f.a_block.stmts, ";", indent + 2);
            nl_indent(fp, indent);
            fprintf(fp, "end");
            break;
            
        case ast_return:
            fprintf(fp, "return(");
            p_a_n(fp, node->f.a_return.expr, indent);
            fprintf(fp, ")");
            break;
            
        case ast_var:
            fprintf(fp, "%s", ste_name(node->f.a_var.var));
            break;
            
        case ast_integer:
            fprintf(fp, "%d", node->f.a_integer.value);
            break;
            
        case ast_float:
            fprintf(fp, "%f", node->f.a_float.value);
            break;
            
        case ast_string:
            fprintf(fp, "\"%s\"", node->f.a_string.string);
            break;
            
        case ast_boolean:
            fprintf(fp, node->f.a_boolean.value ? "true" : "false");
            break;
            
        case ast_times:
        case ast_divide:
        case ast_plus:
        case ast_minus:
        case ast_eq:
        case ast_neq:
        case ast_lt:
        case ast_le:
        case ast_gt:
        case ast_ge:
        case ast_and:
        case ast_or:
        case ast_cand:
        case ast_cor:
            fprintf(fp, "(");
            p_a_n(fp, node->f.a_binary_op.larg, indent);
            
            switch (node->type) {
                case ast_times:   fprintf(fp, " * ");  break;
                case ast_divide:  fprintf(fp, " / ");  break;
                case ast_plus:    fprintf(fp, " + ");  break;
                case ast_minus:   fprintf(fp, " - ");  break;
                case ast_eq:      fprintf(fp, " = ");  break;
                case ast_neq:     fprintf(fp, " != "); break;
                case ast_lt:      fprintf(fp, " < ");  break;
                case ast_le:      fprintf(fp, " <= "); break;
                case ast_gt:      fprintf(fp, " > ");  break;
                case ast_ge:      fprintf(fp, " >= "); break;
                case ast_and:     fprintf(fp, " and "); break;
                case ast_or:      fprintf(fp, " or ");  break;
                case ast_cand:    fprintf(fp, " cand "); break;
                case ast_cor:     fprintf(fp, " cor ");  break;
                default: break;
            }
            
            p_a_n(fp, node->f.a_binary_op.rarg, indent);
            fprintf(fp, ")");
            break;
            
        case ast_not:
            fprintf(fp, "(not ");
            p_a_n(fp, node->f.a_unary_op.arg, indent);
            fprintf(fp, ")");
            break;
            
        case ast_uminus:
            fprintf(fp, "(-");
            p_a_n(fp, node->f.a_unary_op.arg, indent);
            fprintf(fp, ")");
            break;
              case ast_program:
            fprintf(fp, "program");
            nl_indent(fp, indent + 2);
            print_ast_list(fp, node->f.a_program.statements, "", indent + 2);  // Use empty separator
            break;

        case ast_eof:
            fprintf(fp, "EOF");
            break;
            
        default:
            fprintf(fp, "Unknown AST node type: %d", node->type);
            break;
    }
}

// Print a list of AST nodes
static void print_ast_list(FILE *fp, ast_list *list, const char *separator, int indent) {
    for (; list != NULL; list = list->tail) {
        p_a_n(fp, list->head, indent);
        // Only add separator, no newline needed as p_a_n already adds a newline for var_decl nodes
        if (list->tail && (separator[0] != '\0')) {
            fprintf(fp, "%s", separator);
        }
    }
}

// Print a list of symbol table entries
static void print_ste_list(FILE *fp, ste_list *list, const char *prefix, const char *separator, int indent) {
    for (; list != NULL; list = list->tail) {
        fprintf(fp, "%s%s : %s", prefix, ste_name(list->head),
               type_names[ste_var_type(list->head)]);
               
        if (list->tail || indent >= 0) fprintf(fp, "%s", separator);
        if (indent >= 0) nl_indent(fp, indent);
    }
}

// Print a newline and indent
static void nl_indent(FILE *fp, int indent) {
    fprintf(fp, "\n");
    for (int i = 0; i < indent; i++) {
        fprintf(fp, " ");
    }
}



