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
#include "object.h"
#include "memory.h"
#include "table.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Global variable. It's ok to use this approach for simplicity because there is only one vm.
VM vm;

#define POP()       (pop_stack(&vm.stack))
#define PUSH(value) (push_stack(&vm.stack, (value)))

//TODO: Note, this is a memory leak now that some Value are in the heap.
static void reset_stack()
{
    vm.stack.size = 0;
    vm.frame_count = 0;
}


static void runtime_error(const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    fprintf(stderr, "Number of frames: %d\n", vm.frame_count);

    for (int i = vm.frame_count - 1; i >= 0; --i)
    {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->function;
        size_t instruction = frame->ip - frame->function->chunk.code - 1;
        int line = frame->function->chunk.lines.lines[instruction].line;
        fprintf(stderr, "Instruction %zu\n", instruction);
        fprintf(stderr, "[line %d] in script\n", line);

        if (function->name == NULL)
        {
            fprintf(stderr, "script\n");
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    
    printf("\nGlobal HashTable:\n");
    print_hashtable(&vm.globals);
    printf("\n");


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

static bool call(ObjFunction* function, uint32_t arg_count)
{
    if (arg_count != function->arity)
    {
        runtime_error("Expected %d arguments but got %d.", function->arity, arg_count);
        return false;
    }

    if (vm.frame_count == FRAMES_MAX)
    {
        runtime_error("Stack overflow from function calls.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->function = function;
    frame->ip = function->chunk.code;
    frame->slots = &vm.stack.s[vm.stack.size -  1 - arg_count - 1];
    return true;
}

static bool call_value(Value callee, uint32_t arg_count)
{
    if (IS_OBJ(callee))
    {
        switch (OBJ_TYPE(callee))
        {
            case OBJ_FUNCTION:
                return call(AS_FUNCTION(callee), arg_count);
            default:
                break; // non callable object 
        }
    }
    runtime_error("Can only call functions and classes.");
    return false;
}


static bool is_falsey(Value value) 
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}


static void concatenate()
{
    ObjString* b = AS_STRING(POP());
    ObjString* a = AS_STRING(POP());
    ObjString* result = concatenate_string(a, b);

    return PUSH(OBJ_VAL(result));
}


static InterpretResult run()
{
    CallFrame* frame = &vm.frames[vm.frame_count - 1];


    #define READ_BYTE() (*frame->ip++)

    #define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

    #define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
    
    #define READ_CONSTANT_LONG() (frame->function->chunk.constants.values[\
        ((0u | READ_BYTE()) << 8\
             | READ_BYTE())  << 8\
             | READ_BYTE()])
    
    #define READ_STRING() AS_STRING(READ_CONSTANT())
    
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
        disassemble_instruction(&frame->function->chunk, (uint32_t)(frame->ip - frame->function->chunk.code));
        #endif

        uint8_t instruction = READ_BYTE();
        switch (instruction)    
        {
        case OP_ADD: 
        {
            if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
            {
                concatenate();
            }
            else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) 
            { 
                double b = AS_NUMBER(POP()); 
                double a = AS_NUMBER(POP()); 
                PUSH(NUMBER_VAL(a + b));
            } 
            else
            {
                runtime_error("Operands must be numbers"); 
                return INTERPRET_RUNTIME_ERROR;
            }
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
        case OP_SWITCH_EQUAL:
        {
            Value b = POP();
            PUSH(BOOL_VAL(values_equal(peek(0), b)));
            break;
        }
        case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
        case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;

        case OP_PRINT:
        {
            print_value(POP());
            printf("\n");
            break;
        }

        case OP_DEFINE_GLOBAL:
        {
            ObjString* name = READ_STRING();
            set_hashtable(&vm.globals, name, peek(0));
            POP();
            break;
        }
        case OP_GET_GLOBAL:
        {
            ObjString* name = READ_STRING();
            Value value;
            if (!get_hashtable(&vm.globals, name, &value))
            {
                runtime_error("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            PUSH(value);
            break;
        }
        case OP_SET_GLOBAL:
        {
            ObjString* name = READ_STRING();
            // True if name is a new key.
            if (set_hashtable(&vm.globals, name, peek(0)))
            {
                del_hashtable(&vm.globals, name);
                runtime_error("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_SET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = peek(0);
            break;
        }
        case OP_GET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            PUSH(frame->slots[slot]);
            break;
        }
        

        case OP_POP: POP(); break;

        
        case OP_JUMP_IF_FALSE:
        {
            uint16_t offset = READ_SHORT();
            if (is_falsey(peek(0)))
            {
                frame->ip += offset;
            }
            break;
        }
        case OP_JUMP:
        {
            uint16_t offset = READ_SHORT();
            frame->ip += offset;
            break;
        }
        case OP_LOOP:
        {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
            break;
        }

        case OP_CALL:
        {
            uint32_t arg_count = READ_BYTE();
            frame->ip = vm.ip; // save current ip in the current frame
            if (!call_value(peek(arg_count), arg_count))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frame_count - 1];
            vm.ip = frame->ip;
            break;
        }
        case OP_RETURN:
        {
            Value result = POP();
            --vm.frame_count;
            if (vm.frame_count == 0)
            {
                POP();
                return INTERPRET_OK;
            }

            vm.stack.size = frame->slots - vm.stack.s + 1;
            PUSH(result);
            frame = &vm.frames[vm.frame_count - 1];
            vm.ip = frame->ip;
            break;
        }


        default:
            break;
        }
    }


    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef READ_CONSTANT_LONG
    #undef READ_STRING
    #undef BINARY_OP
}


void init_vm()
{
    init_stack(&vm.stack);
    vm.objects = NULL;
    init_hashtable(&vm.globals);
    init_hashtable(&vm.strings);
}


InterpretResult interpret(const char* source)
{
    ObjFunction* function = compile(source);
    if (function == NULL)
    {
        return INTERPRET_COMPILE_ERROR;
    }

    PUSH(OBJ_VAL(function));
    call(function, 0);

    return run();
}

void free_vm()
{
    free_hashtable(&vm.globals);
    free_hashtable(&vm.strings);
    free_objects();
    free_stack(&vm.stack);
}

#undef POP