/*
c/lox/compiler.h
*/
#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"
#include "value.h"
#include "object.h"

ObjFunction* compile(const char* source);

#endif