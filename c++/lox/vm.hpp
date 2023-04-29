/*
c++/lox/vm.hpp

PURPOSE:

CLASSES:

*/

#ifndef VM_HPP
#define VM_HPP

#include "value.hpp"
#include "chunk.hpp"
#include "common.hpp"


namespace lox
{
    enum class InterpretResult
    {
        InterpretOk,
        InterpretCompileError,
        InterpretRuntimeError,
    };


    class VM
    {
    public:
        VM(non_nullable_res<Chunk> chunk_) : chunk(chunk_)
        {
            
        }

        auto Interpret() -> InterpretResult;

    private:
        Stack<Value> stack;
        
        non_owned_res<Chunk> chunk;

        // Instruction pointer
        std::vector<u8>::const_iterator ip; 
    };
} // namespace lox



#endif 