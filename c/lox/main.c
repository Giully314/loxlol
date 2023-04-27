#include <stdio.h>

#include "chunk.h"
#include "value.h"
#include "debug.h"
#include "common.h"
#include "vm.h"

int main()
{
    init_vm();

    Chunk chunk;
    init_chunk(&chunk);

    // uint8_t c = add_constant(&chunk, 3.14);
    // write_chunk(&chunk, OP_CONSTANT, 1);
    // write_chunk(&chunk, c, 1);


    write_constant(&chunk, 4, 1);
    write_constant(&chunk, 3, 2);
    write_constant(&chunk, 2, 2);
    write_chunk(&chunk, OP_NEGATE, 2);
    write_chunk(&chunk, OP_MULTIPLY, 2);
    write_chunk(&chunk, OP_SUBTRACT, 2);
    
    // write_constant(&chunk, 5, 2);
    
    // write_chunk(&chunk, OP_DIVIDE, 2);
    // write_chunk(&chunk, OP_ADD, 2);
    // write_chunk(&chunk, OP_ADD, 2);


    write_chunk(&chunk, OP_RETURN, 3);



    interpret(&chunk);
    
    free_vm();
    
    free_chunk(&chunk);
    return 0;
}