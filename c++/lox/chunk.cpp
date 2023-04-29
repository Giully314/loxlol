/*
c++/lox/chunk.cpp
*/

#include "chunk.hpp"
#include "common.hpp"
#include "opcodes.hpp"
#include "value.hpp"

#include <iostream>
namespace lox
{

    // *************************** PUBLIC API *****************************************


    auto Chunk::WriteConstant(const Value& value, u32 line) -> void
    {
        values.push_back(value); 
        u32 idx = values.size() - 1;

        if (idx < MaxConstantOperands) [[likely]]
        {
            WriteOpcode(OpCode::Constant, line);
            WriteOperandIndex(static_cast<u8>(idx), line);
        }
        else
        {
            WriteOpcode(OpCode::ConstantLong, line);
            WriteOperandIndex(idx, line);
        }
    }




    // ***************************** PRIVATE ********************************************

    auto Chunk::AddLine(u32 offset, u32 line) -> void
    {
        // This is executed only the first time to initialize the vector.
        if (lines.empty()) [[unlikely]]
        {
            lines.emplace_back(offset, line);
            return;
        }
        
        // Check if we are still in the same line.
        if (lines.back().second == line) [[likely]]
        {
            return;
        }

        // Insert the idx of the first opcode that starts the new line.
        lines.emplace_back(offset, line);
    }


    auto Chunk::WriteOperandIndex(u32 operand_idx, u32 line) -> void
    {
        u8 c1 = operand_idx & 0xFF;
        operand_idx = operand_idx >> 8;
        u8 c2 = operand_idx & 0xFF;
        operand_idx = operand_idx >> 8;
        u8 c3 = operand_idx & 0xFF;
        operand_idx = operand_idx >> 8;

        // Big endian insert.

        // Check if it is a new line with the first byte of the operand idx.
        code.push_back(c3);
        AddLine(code.size() - 1, line);

        code.push_back(c2);
        code.push_back(c1);
    }

} // namespace lox
