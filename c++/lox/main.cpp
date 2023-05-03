
#include "opcodes.hpp"
#include "chunk.hpp"
#include "value.hpp"
#include "debug.hpp"
#include "common.hpp"
#include "vm.hpp"
#include "token.hpp"

int main()
{  
    using namespace lox;

    Chunk chunk;
    VM vm{&chunk};

    chunk.WriteConstant(4, 1);
    chunk.WriteConstant(3, 1);
    chunk.WriteConstant(2, 1);
    chunk.WriteOpcode(OpCode::Negate, 1);
    chunk.WriteOpcode(OpCode::Multiply, 1);
    chunk.WriteOpcode(OpCode::Subtract, 1);
    chunk.WriteOpcode(OpCode::Return, 32);
    
    vm.Interpret();

    return 0;
}