/*
c++/lox/compiler.hpp

PURPOSE:

CLASSES:

DESCRIPTION:
*/

#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "common.hpp"
#include "scanner.hpp"

#include <string>
#include <string_view> 

namespace lox
{
    class Compiler
    {
    public:
        Compiler(std::string_view source_) : source(source_), scanner(source_)
        {

        }


        auto Compile() -> void;
    
    private:
        std::string_view source;
        Scanner scanner;
    };
} // namespace lox


#endif