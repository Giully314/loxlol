/*
c++/lox/opcodes.hpp

PURPOSE:
    Define the byte codes executed by the lox virtual machine.
*/

#ifndef OPCODES_HPP
#define OPCODES_HPP

#include "common.hpp"

namespace lox
{
    enum class OpCode : u8
    {
        // Binary 
        Add = 0,
        Subtract,
        Multiply,
        Divide,

        // Unary 
        Negate,

        // Constants
        ConstantLong, // support 24 bit operand for constant
        Constant,
        
        Return,
    };
} // namespace lox


#endif 