/*
c++/lox/debug.cpp
*/

#include "debug.hpp"
#include "common.hpp"
#include "opcodes.hpp"

#include <iostream>
#include <format>

namespace lox
{
    // ******************************** PUBLIC ***********************************

    auto Debug::PrintStack(const Stack<Value>& stack) -> void
    {
        std::cout << "          ";
        for (const auto& v : stack)
        {
            std::cout << "[ ";
            PrintValue(v);
            std::cout << " ]";
        }
        std::cout << std::endl;
    }

    auto Debug::DisassembleChunk(const Chunk& chunk, std::string_view name) -> void
    {
        std::cout << std::format("== {} ==", name);

        for (u32 offset = 0; offset < chunk.Size();)
        {
            DisassembleInstruction(chunk, offset);
        }
    }


    auto Debug::DisassembleInstruction(const Chunk& chunk, u32 offset) -> u32
    {
        // printf("%04u ", offset);

        std::cout << std::format("{:04}", offset);

        // Note: The GetLine(offset-1) could be avoided if we save the prev line.
        u32 line = chunk.GetLine(offset);
        if (offset > 0 && line == chunk.GetLine(offset-1)) // Same line
        {
            std::cout << "   | ";
        }
        else
        {
            std::cout << std::format("{:4} ", line);
        }

        OpCode instruction = static_cast<OpCode>(chunk[offset]);
        switch (instruction)
        {
            case OpCode::Add:
                return SimpleInstruction("OP_ADD", offset);
            case OpCode::Subtract:
                return SimpleInstruction("OP_SUBTRACT", offset);
            case OpCode::Multiply:
                return SimpleInstruction("OP_MULTIPLY", offset);
            case OpCode::Divide:
                return SimpleInstruction("OP_DIVIDE", offset);
                
            case OpCode::Negate:
                return SimpleInstruction("OP_NEGATE", offset);
            
            case OpCode::ConstantLong:
                return ConstantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
            case OpCode::Constant:
                return ConstantInstruction("OP_CONSTANT", chunk, offset);
            
            case OpCode::Return:
                return SimpleInstruction("OP_RETURN", offset);
            default:
                std::cout << "Unkwown opcode" << std::endl;
                return offset + 1;
        }
    }
    
    auto Debug::PrintValue(const Value& value) -> void
    {
        std::cout << std::format("{:g}", value);
    }

    // ******************************** PRIVATE ***********************************

    auto Debug::ConstantInstruction(std::string_view name, const Chunk& chunk, u32 offset) -> u32
    {
        u8 constant = chunk[offset + 1];
        std::cout << std::format("{:16} {:04} '", name, constant);
        PrintValue(chunk.GetValues()[constant]);
        std::cout << "'\n";
        return offset + 2;
    }

    auto Debug::ConstantLongInstruction(std::string_view name, const Chunk& chunk, u32 offset) -> u32
    {
        // Extract the 3 bytes.
        u32 constant = 0;
        constant = constant | chunk[offset + 1]; 
        constant = constant << 8;
        constant = constant | chunk[offset + 2]; 
        constant = constant << 8;
        constant = constant | chunk[offset + 3]; 

        // printf("%-16s %4u '", name, constant);
        std::cout << std::format("{:16} {:04} '", name, constant);
        PrintValue(chunk.GetValues()[constant]);
        std::cout << "'" << std::endl;
        return offset + 4;
    }

    auto Debug::SimpleInstruction(std::string_view name, u32 offset) -> u32
    {
        std::cout << name << "\n";
        return offset + 1;
    }


} // namespace lox