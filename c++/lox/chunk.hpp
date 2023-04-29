/*
c++/lox/chunk.hpp

PURPOSE:
    Manage bytecodes.


CLASSES: 
    Chunk: Define a chunk of byte codes and operations to manage them.

DESCRIPTION:
    A chunk is just an array of byte codes and operands. A subset of byte codes operate on one or more
    operands and so this class manages store these informations. 
    The order of execution is FIFO.
*/


#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "common.hpp"
#include "value.hpp"
#include "opcodes.hpp"

#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <format>

namespace lox
{

    class Chunk
    {
    public:
        Chunk()
        {
            // Initialize with line 0 to avoid checks on the size when writing a new line.
            // Without this we should always check if lines is not empty before calling lines.back().
            // lines.emplace_back(0, 0);
        }

        // Access the code array at itx.
        // Precondition: 0 <= idx < code.size(). 
        auto operator[](u32 idx) const -> u8
        {   
            return code[idx];
        }

        auto Size() const noexcept -> u32
        {
            return code.size();
        }


        auto WriteOpcode(OpCode op, u32 line) -> void
        {
            code.push_back(static_cast<u8>(op));
            AddLine(code.size() - 1, line);
        }


        auto WriteConstant(const Value& value, u32 line) -> void;   


        auto GetCode() const noexcept -> const std::vector<u8>&
        {
            return code;
        }

        auto GetValues() const noexcept -> const std::vector<Value>&
        {
            return values;
        }


        // Precondition: offset must be a valid idx in the code array.
        // Complexity: O(lg(n))
        auto GetLine(u32 offset) const -> u32
        {
            auto it = std::ranges::upper_bound(lines, offset, {}, [](auto&& e){return e.first;});
            return std::prev(it)->second;
        }   

    private:
        auto AddLine(u32 offset, u32 line) -> void; 


        auto WriteOperandIndex(u8 operand_idx, u32 line) -> void
        {
            code.push_back(operand_idx);
            AddLine(code.size() - 1, line);
        }
        
        auto WriteOperandIndex(u32 operand_idx, u32 line) -> void;

    private:
        // first: idx to first opcode that starts a new line.
        // second: the line number.
        using Line = std::pair<u32, u32>;

        // Byte codes.
        std::vector<u8> code;

        // Constant values used as operands.
        std::vector<Value> values;

        // Map opcodes to lines.
        std::vector<Line> lines;

        // OpCode::Constant uses an 8bit operand idx. If we go beyond this number we need to use ConstantLong.
        static inline constexpr u16 MaxConstantOperands = 256; 
    };

} // namespace lox


#endif