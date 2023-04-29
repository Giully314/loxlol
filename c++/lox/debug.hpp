/*
c++/lox/debug.hpp

PURPOSE:
    Provide utility functions for debug.

*/

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "chunk.hpp"
#include "common.hpp"
#include "value.hpp"

#include <stack>
#include <string_view>


#define DEBUG_TRACE_EXECUTION

namespace lox
{
    struct Debug
    {
        static auto DisassembleChunk(const Chunk& chunk, std::string_view name) -> void;
        static auto DisassembleInstruction(const Chunk& chunk, u32 offset) -> u32;
        static auto PrintStack(const Stack<Value>& stack) -> void;
        static auto PrintValue(const Value& value) -> void;
    
    private:
        static auto ConstantInstruction(std::string_view name, const Chunk& chunk, u32 offset) -> u32;
        static auto ConstantLongInstruction(std::string_view name, const Chunk& chunk, u32 offset) -> u32;
        static auto SimpleInstruction(std::string_view name, u32 offset) -> u32;
    };

} // namespace lox


#endif 