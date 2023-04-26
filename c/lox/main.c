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

    write_constant(&chunk, 3.14, 1);

    write_chunk(&chunk, OP_RETURN, 1);
    write_chunk(&chunk, OP_RETURN, 1);
    write_chunk(&chunk, OP_RETURN, 1);
    write_chunk(&chunk, OP_RETURN, 2);
    write_chunk(&chunk, OP_RETURN, 3);
    write_chunk(&chunk, OP_RETURN, 4);
    
    write_constant(&chunk, 2.7, 5);
    write_constant(&chunk, 12, 5);
    write_chunk(&chunk, OP_RETURN, 5);

    disassemble_chunk(&chunk, "test chunk");
    
    free_vm();
    
    free_chunk(&chunk);
    return 0;
}