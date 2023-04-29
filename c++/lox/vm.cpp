/*
c++/lox/vm.cpp
*/

#include "vm.hpp"
#include "opcodes.hpp"
#include "debug.hpp"

#include <iterator>
#include <iostream>

namespace lox
{

    auto VM::Interpret() -> InterpretResult
    {
        #define READ_BYTE() (*ip++)
        #define READ_CONSTANT() (chunk->GetValues()[READ_BYTE()])
        #define READ_CONSTANT_LONG() (chunk->GetValues()[\
            ((0u | READ_BYTE()) << 8\
                | READ_BYTE())  << 8\
                | READ_BYTE()])

        #define BINARY_OP(op) \
            do { \
                Value b = stack.back(); \
                stack.pop_back(); \
                stack.back() op##= b;\
            } while (false)

        ip = chunk->GetCode().cbegin();

        while (true)
        {
            #ifdef DEBUG_TRACE_EXECUTION
            Debug::PrintStack(stack);
            Debug::DisassembleInstruction(*chunk, std::distance(chunk->GetCode().cbegin(), ip));
            #endif

            OpCode instruction = static_cast<OpCode>(READ_BYTE());
            switch (instruction)    
            {
            // Binary.
            case OpCode::Add: 
            {
                BINARY_OP(+);
                break;
            }
            case OpCode::Subtract: 
            {
                BINARY_OP(-);
                break;
            }
            case OpCode::Multiply: 
            {
                BINARY_OP(*);
                break;
            }
            case OpCode::Divide: 
            {
                BINARY_OP(/);
                break;
            }

            // Unary 
            case OpCode::Negate: 
            {
                stack.back() = -stack.back();
                break;
            }  

            // Constant
            case OpCode::ConstantLong:
            {
                Value value = READ_CONSTANT_LONG();
                stack.push_back(value);
                break;
            }
            case OpCode::Constant:
            {
                Value value = READ_CONSTANT();
                stack.push_back(value);
                break;
            }

            case OpCode::Return:
            {
                Debug::PrintValue(stack.back());
                stack.pop_back();
                std::cout << std::endl;
                return InterpretResult::InterpretOk;
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

} // namespace lox