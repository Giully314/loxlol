/*
c/lox/compiler.c
*/
#include "compiler.h"
#include "common.h"
#include "scanner.h"
#include "debug.h"
#include "value.h"
#include "object.h"
#include "chunk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct
{
    Token previous;
    Token current;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum 
{
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    //PREC_CONDITIONAL,     // ?:
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;


typedef void(*ParseFn)(bool can_assign);

typedef struct 
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;


typedef struct 
{
    Token name;
    int depth;
} Local;


typedef struct
{
    Local locals[UINT8_COUNT];
    uint32_t local_count;
    uint32_t scope_depth;
} Compiler;



// Global instance.
Parser parser;
Chunk* compiling_chunk;
Compiler* current = NULL;


static void init_compiler(Compiler* compiler)
{
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    current = compiler;
}



static Chunk* current_chunk()
{
    return compiling_chunk;
}



// ******************** ERROR ******************************************************


static void error_at(Token* token, const char* msg)
{
    parser.panic_mode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) 
    {
        fprintf(stderr, " at end");
    } 
    else if (token->type == TOKEN_ERROR) 
    {
        // Nothing.
    } 
    else 
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", msg);
    parser.had_error = true;
}


static void error(const char* msg)
{
    error_at(&parser.previous, msg);
}

static void error_at_current(const char* msg)
{
    error_at(&parser.current, msg);
}

// ******************** ERROR ******************************************************


// **************************** PARSER **********************************************



static void advance()
{
    parser.previous = parser.current;

    while (true)
    {
        parser.current = scan_token();
        if (parser.current.type != TOKEN_ERROR)
        {
            break;
        }
        error_at_current(parser.current.start);
    }
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}


static bool match(TokenType type)
{
    if (!check(type))
    {
        return false;
    }

    advance();
    return true;
}


static void consume(TokenType type, const char* msg)
{
    if (parser.current.type == type) 
    {
        advance();
        return;
    }

    error_at_current(msg);
}


static void synchronize() 
{
    parser.panic_mode = false;

    while (parser.current.type != TOKEN_EOF) 
    {
        if (parser.previous.type == TOKEN_SEMICOLON) 
        {
            return;
        }

        switch (parser.current.type) 
        {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:
                ; // Do nothing.
        }

        advance();
    }
}


static void emit_byte(uint8_t byte)
{
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2)
{
    emit_byte(byte1);
    emit_byte(byte2);
}

static void emit_return()
{
    emit_byte(OP_RETURN);
}


static uint32_t emit_jump(uint8_t instruction)
{
    emit_byte(instruction);
    emit_bytes(0xff, 0xff); // placeholder for the jump offset.
    return current_chunk()->size - 2;
}

static void emit_loop(uint32_t loop_start)
{
    emit_byte(OP_LOOP);

    // + 2 to take into account the OP_LOOP instruction's operand
    uint32_t offset = current_chunk()->size - loop_start + 2;
    if (offset > UINT16_MAX)
    {
        error("Loop body too large.");
    }

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

static void patch_jump(uint32_t offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    uint32_t jump = current_chunk()->size - offset - 2;
    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over.");
    }

    Chunk* chunk = current_chunk();
    chunk->code[offset] =   (jump >> 8) & 0xff;
    chunk->code[offset+1] = jump & 0xff;
}


static uint8_t make_constant(Value value)
{
    uint32_t constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Value value)
{
    emit_bytes(OP_CONSTANT, make_constant(value));
}



// **************************** PARSER **********************************************


// ************************** COMPILER **********************************************+

// Note: the assumption when calling functions linked to token type is that the token is stored in previous.

static void end_compiler()
{
    emit_return();
    #ifdef DEBUG_PRINT_CODE
    if (!parser.had_error)
    {
        disassemble_chunk(current_chunk(), "code");
    }
    #endif
}


static void begin_scope()
{
    ++current->scope_depth;
}

static void end_scope()
{
    --current->scope_depth;

    while (current->local_count > 0 && current->locals[current->local_count-1].depth > current->scope_depth)
    {
        emit_byte(OP_POP);
        --current->local_count;
    }
}

static int resolve_local(Compiler* compiler, Token* name);
static uint8_t identifier_constant(Token* name);
static void define_variable(uint8_t global);
static uint8_t parse_variable(const char* error_message);
static void expression();
static void statement();
static void declaration();
static ParseRule* get_rule(TokenType type);
static void parse_precedence(Precedence precedence);


static void expression()
{
    parse_precedence(PREC_ASSIGNMENT);
}

static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void var_declaration()
{
    uint8_t global = parse_variable("Expect variable name.");

    if (match(TOKEN_EQUAL))
    {
        expression();   
    }
    else
    {
        emit_byte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");

    define_variable(global);
}

static void expression_statement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emit_byte(OP_POP);
}

// ************************************ Exercise 23.1 ************************************

static uint32_t default_statement()
{
    // consume(TOKEN_DEFAULT, "Expect 'case' for switch");
    advance();
    consume(TOKEN_COLON, "Expect ':' after case expression.");
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        statement();
    }
    uint32_t end = emit_jump(OP_JUMP);
    return end;
}

static uint32_t case_statement()
{
    consume(TOKEN_CASE, "Expect 'case' for switch");
    expression();
    consume(TOKEN_COLON, "Expect ':' after case expression.");
    emit_byte(OP_SWITCH_EQUAL);
    uint32_t next = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP); // if true

    while (!check(TOKEN_CASE) && !check(TOKEN_DEFAULT) && !check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        statement();
    }
    uint32_t end = emit_jump(OP_JUMP);
    patch_jump(next);
    emit_byte(OP_POP);
    return end;
}

static void switch_statement()
{
    // TODO: add better errors
    // - default must be the last 
    // - max number of cases is 32.
    consume(TOKEN_LEFT_PAREN, "Expect '(' after switch.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after switch condition.");
    
    consume(TOKEN_LEFT_BRACE, "Expect '{' after switch condition.");

    // Max 32 number of cases
    // TODO: check for idx overflow.
    uint32_t cases[32];
    uint32_t idx = 0;
    // case statement 
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_DEFAULT) && !check(TOKEN_EOF))
    {
        cases[idx++] = case_statement();
    }

    // default statement
    if (check(TOKEN_DEFAULT))
    {
        cases[idx++] = default_statement();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after switch condition.");
    
    for (uint32_t i = 0; i < idx; ++i)
    {
        patch_jump(cases[i]);
    }

    emit_byte(OP_POP); // pop the value of expression condition
}

// ************************************ Exercise 23.1 ************************************

static void if_statement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after if.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after if condition.");
    
    uint32_t then_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP); // Clean the stack from the value of the condition (if true).
    statement();

    uint32_t else_jump = emit_jump(OP_JUMP);
    
    patch_jump(then_jump);
    emit_byte(OP_POP); // Clean the stack from the value of the condition (if false).

    if (match(TOKEN_ELSE))
    {
        statement();
    }
    patch_jump(else_jump);
}

static void print_statement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emit_byte(OP_PRINT);
}

static void while_statement()
{
    uint32_t loop_start = current_chunk()->size;
    consume(TOKEN_LEFT_PAREN, "Expect '(' after while.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after while condition.");

    uint32_t exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();

    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_byte(OP_POP);
}

static void for_statement()
{
    begin_scope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after for.");
    
    if (match(TOKEN_SEMICOLON))
    {
        // No initializer.
    }
    else if (match(TOKEN_VAR))
    {
        var_declaration();
    }
    else
    {
        expression_statement();
    }


    uint32_t loop_start = current_chunk()->size;
    int exit_jump = -1;
    if (!match(TOKEN_SEMICOLON)) // Check condition
    {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';'.");
    
        exit_jump = emit_jump(OP_JUMP_IF_FALSE);
        emit_byte(OP_POP);
    }

    if (!match(TOKEN_RIGHT_PAREN))
    {
        uint32_t body_jump = emit_jump(OP_JUMP);
        uint32_t increment_start = current_chunk()->size;
        expression();
        emit_byte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for condition.");

        emit_loop(loop_start);
        loop_start = increment_start;
        patch_jump(body_jump);
    }


    statement();
    emit_loop(loop_start);
    
    if (exit_jump != -1)
    {
        patch_jump(exit_jump);
        emit_byte(OP_POP);
    }

    end_scope();
}

static void declaration()
{
    if (match(TOKEN_VAR))
    {
        var_declaration();
    }
    else
    {
        statement();
    }

    if (parser.panic_mode)
    {
        synchronize();
    }
}

static void statement()
{
    if (match(TOKEN_PRINT))
    {
        print_statement();
    }
    else if (match(TOKEN_LEFT_BRACE))
    {
        begin_scope();
        block();
        end_scope();
    }
    else if (match(TOKEN_IF))
    {
        if_statement();
    }
    else if (match(TOKEN_WHILE))
    {
        while_statement();
    }
    else if (match(TOKEN_FOR))
    {
        for_statement();
    }
    else if (match(TOKEN_SWITCH))
    {
        switch_statement();
    }
    else
    {
        expression_statement();
    }
}


static void grouping(bool can_assign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}


static void number(bool can_assign)
{
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void and_(bool can_assign)
{
    uint32_t end_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    parse_precedence(PREC_AND);
    patch_jump(end_jump);
}

static void or_(bool can_assign)
{
    uint32_t else_jump = emit_jump(OP_JUMP_IF_FALSE);
    uint32_t end_jump = emit_jump(OP_JUMP);

    patch_jump(else_jump);
    emit_byte(OP_POP);

    parse_precedence(PREC_OR);
    patch_jump(end_jump);
}


static void string(bool can_assign)
{
    emit_constant(OBJ_VAL(copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

static void named_variable(Token name, bool can_assign)
{
    uint8_t get_op, set_op;

    int arg = resolve_local(current, &name);
    if (arg != -1)
    {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    }
    else
    {
        arg = identifier_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && match(TOKEN_EQUAL))
    {
        expression();
        emit_bytes(set_op, (uint8_t)arg);
    }
    else
    {
        emit_bytes(get_op, (uint8_t)arg);
    }
}

static void variable(bool can_assign)
{
    named_variable(parser.previous, can_assign);
}


static void literal(bool can_assign) 
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE:   emit_byte(OP_FALSE); break;
    case TOKEN_NIL:     emit_byte(OP_NIL); break;
    case TOKEN_TRUE:    emit_byte(OP_TRUE); break;
    default: return; // Unreachable.
    }
}


static void unary(bool can_assign)
{
    TokenType operator_type = parser.previous.type;

    // Compile the operand.
    parse_precedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operator_type)
    {
        case TOKEN_MINUS:
        {
            emit_byte(OP_NEGATE);
            break;
        }
        case TOKEN_BANG:
        {
            emit_byte(OP_NOT);
            break;
        }
        default:
            return;
    }
}


static void binary(bool can_assign)
{
    TokenType operator_type = parser.previous.type;
    ParseRule* rule = get_rule(operator_type);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (operator_type)
    {
        case TOKEN_PLUS:
        {
            emit_byte(OP_ADD);
            break;
        }
        case TOKEN_MINUS:
        {
            emit_byte(OP_SUBTRACT);
            break;
        }
        case TOKEN_STAR:
        {
            emit_byte(OP_MULTIPLY);
            break;
        }
        case TOKEN_SLASH:
        {
            emit_byte(OP_DIVIDE);
            break;
        }
        case TOKEN_BANG_EQUAL:    emit_bytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emit_byte(OP_EQUAL); break;
        case TOKEN_GREATER:       emit_byte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emit_byte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emit_bytes(OP_GREATER, OP_NOT); break;

        default: return; // Unreachable.
    }
}




ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {variable,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     or_,   PREC_OR},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};



static void parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL)
    {
        error("Expect expression");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TOKEN_EQUAL)) 
    {
        error("Invalid assignment target.");
    }
}

static uint8_t identifier_constant(Token* name) 
{
    return make_constant(OBJ_VAL(copy_string(name->start,
                                         name->length)));
}

static bool identifiers_equal(Token* a, Token* b)
{
    if (a->length != b->length)
    {
        return false;
    }

    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolve_local(Compiler* compiler, Token* name)
{
    for (int i = compiler->local_count - 1; i >= 0; --i)
    {
        Local* local = &compiler->locals[i];
        if (identifiers_equal(name, &local->name))
        {
            if (local->depth == -1)
            {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}


static void add_local(Token name)
{
    if (current->local_count == UINT8_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }
    Local* local = &current->locals[current->local_count++];
    local->name = name;
    local->depth = -1;
}

static void declare_variable()
{
    if (current->scope_depth == 0)
    {
        return;
    }

    Token* name = &parser.previous;

    // Check if we have 2 variables with the same name in the same scope.
    for (int i = current->local_count - 1; i >= 0; --i)
    {
        Local* local = &current->locals[i];

        if (local->depth != -1 && local->depth < current->scope_depth)
        {
            break;
        }

        if (identifiers_equal(name, &local->name))
        {
            error("Already a variable in this scope.");
        }
    }
    
    add_local(*name);
}

static uint8_t parse_variable(const char* error_message) 
{
    consume(TOKEN_IDENTIFIER, error_message);

    declare_variable();
    if (current->scope_depth > 0) return 0;

    return identifier_constant(&parser.previous);
}

static void mark_initialized()
{
    current->locals[current->local_count - 1].depth = current->scope_depth;
}

static void define_variable(uint8_t global) 
{
    // If we the variable is local, we have already it in the stack after the parsing.
    if (current->scope_depth > 0)
    {
        mark_initialized();
        return;
    }
    // Emits a bytecode only for global variable! 
    emit_bytes(OP_DEFINE_GLOBAL, global);
}


static ParseRule* get_rule(TokenType type) 
{
    return &rules[type];
}



bool compile(const char* source, Chunk* chunk)
{
    init_scanner(source);
    Compiler compiler;
    init_compiler(&compiler);
    compiling_chunk = chunk;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();
    
    while (!match(TOKEN_EOF))
    {
        declaration();
    }

    end_compiler();

    return !parser.had_error;
}

