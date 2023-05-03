/*
c++/lox/token.hpp

PURPOSE:

*/

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "common.hpp"

#include <string_view>


namespace lox
{
    enum class TokenType
    {
        // Single-character tokens.
        LeftParen, RightParen,
        LeftBrace, RightBrace,
        Comma,      Dot,        Minus, Plus,
        Semicolon,  Slash,      Star,
        // One or two character tokens.
        Bang,    BangEqual,
        Equal,   EqualEqual,
        Greater, GreaterEqual,
        Less,    LessEqual,
        // Literals.
        Identifier, String, Number,
        // Keywords.
        And,   Class,  Else,  False,
        For,   Fun,    If,    Nil, Or,
        Print, Return, Super, This,
        True,  Var,    While,

        Error, Eof, 
    };

    struct Token
    {   
        Token() = default;
        
        Token(TokenType type_, std::string_view start_, u32 line_) : type(type_), start(start_), line(line_)
        {

        }

        TokenType type;
        // pointer to the source code
        std::string_view start;
        u32 line;
    };
} // namespace lox



#endif 