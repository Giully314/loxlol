/*
c/lox/compiler.h
*/
#ifndef COMPILER_H
#define COMPILER_H

#include "chunk.h"
#include "common.h"

bool compile(const char* source, Chunk* chunk);

#endif