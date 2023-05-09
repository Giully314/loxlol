/*
lox/vm.c
*/

#include "vm.h"
#include "chunk.h"
#include "value.h"
#include "common.h"
#include "debug.h"
#include "stack.h"
#include "compiler.h"

#include <stdio.h>
#include <stdarg.h>

// Global variable. It's ok to use this approach for simplicity because there is only one vm.
VM vm;

static void reset_stack()
{
    vm.stack.size = 0;
}


static void runtime_error(const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines.lines[instruction].line;
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

static void print_stack()
{
    printf("          ");
    for (uint32_t i = 0; i < vm.stack.size; ++i)
    {
        printf("[ ");
        print_value(vm.stack.s[i]);
        printf(" ]");
    }
    printf("\n");
}


static Value peek(int distance) 
{
    return vm.stack.s[vm.stack.size - 1 - distance];
}


static bool is_falsey(Value value) 
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}


static InterpretResult run()
{
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    #define READ_CONSTANT_LONG() (vm.chunk->constants.values[\
        ((0u | READ_BYTE()) << 8\
             | READ_BYTE())  << 8\
             | READ_BYTE()])

    #define BINARY_OP(value_type, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
            { \
                runtime_error("Operands must be numbers"); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(pop_stack(&vm.stack)); \
            double a = AS_NUMBER(pop_stack(&vm.stack)); \
            push_stack(&vm.stack, value_type(a op b)); \
        } while (false)

            // double a = pop_stack(&vm.stack); 
            //push_stack(&vm.stack, a op b); 


    while (true)
    {
        #ifdef DEBUG_TRACE_EXECUTION
        print_stack();
        disassemble_instruction(vm.chunk, (uint32_t)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction = READ_BYTE();
        switch (instruction)    
        {
        case OP_ADD: 
        {
            BINARY_OP(NUMBER_VAL, +);
            break;
        }
        case OP_SUBTRACT: 
        {
            BINARY_OP(NUMBER_VAL, -);
            break;
        }
        case OP_MULTIPLY: 
        {
            BINARY_OP(NUMBER_VAL, *);
            break;
        }
        case OP_DIVIDE: 
        {
            BINARY_OP(NUMBER_VAL, /);
            break;
        }

        case OP_NOT:
        {
            push_stack(&vm.stack, BOOL_VAL(is_falsey(pop_stack(&vm.stack))));
            break;
        }

        case OP_NEGATE: 
        {
            if (!IS_NUMBER(peek(0)))
            {
                runtime_error("Operand must be a number");
                return INTERPRET_RUNTIME_ERROR;
            }
            uint32_t top = vm.stack.size - 1;
            vm.stack.s[top] = NUMBER_VAL(-AS_NUMBER(vm.stack.s[top]));
            break;
        }
        case OP_CONSTANT_LONG:
        {
            Value value = READ_CONSTANT_LONG();
            push_stack(&vm.stack, value);
            break;
        }
        case OP_CONSTANT:
        {
            Value value = READ_CONSTANT();
            push_stack(&vm.stack, value);
            break;
        }
        case OP_NIL:    push_stack(&vm.stack, NIL_VAL); break;
        case OP_TRUE:   push_stack(&vm.stack, BOOL_VAL(true)); break;
        case OP_FALSE:  push_stack(&vm.stack, BOOL_VAL(false)); break;
        case OP_EQUAL: 
        {
            Value b = pop_stack(&vm.stack);
            Value a = pop_stack(&vm.stack);
            push_stack(&vm.stack, BOOL_VAL(values_equal(a, b)));
            break;
        }
        case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
        case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;

        case OP_RETURN:
        {
            print_value(pop_stack(&vm.stack));
            printf("\n");
            return INTERPRET_OK;
        }
        
        default:
            break;
        }
    }


    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef READ_CONSTANT_LONG
    #undef BINARY_OP
}


void init_vm()
{
    init_stack(&vm.stack);
}


InterpretResult interpret(const char* source)
{
    Chunk chunk;
    init_chunk(&chunk);

    if (!compile(source, &chunk))
    {
        free_chunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    free_chunk(&chunk);

    return result;
}

void free_vm()
{
    free_stack(&vm.stack);
}
