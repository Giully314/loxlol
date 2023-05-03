/*
c++/lox/scanner.hpp

PURPOSE:

CLASSES:

DESCRIPTION:
*/

#ifndef SCANNER_HPP
#define SCANNER_HPP


#include "token.hpp"

#include <cctype>
#include <string_view>


namespace lox
{
    class Scanner
    {
    public:
        // Scanner() = default;
        Scanner(std::string_view source_) : source(source_), start(source.cbegin()), current(start)
        {

        }

        // auto SetSource(std::string_view source)
        // {
        //     this->source = source;
        //     start = source.cbegin();
        //     current = start;
        // }

        auto ScanToken() -> Token;


    private:
        auto MakeToken(TokenType type) -> Token
        {
            return Token{type, std::string_view{start, current}, current_line};
        }

        auto ErrorToken(std::string_view msg) -> Token
        {
            return Token{TokenType::Error, msg, current_line};
        } 

    
        auto IsAtEnd() const noexcept -> bool
        {
            return source.cend() == current;
        }

        auto Peek() const noexcept -> char
        {
            return *current;
        }

        auto PeekNext() const noexcept -> char
        {
            if (IsAtEnd())
            {
                return '\0';
            }
            return *(current + 1);
        }

        auto Advance() noexcept -> char
        {
            return *current++;
        }

        auto Match(char expected) -> bool
        {
            if (IsAtEnd()) return false;
            if (*current != expected) return false;
            ++current;
            return true;
        }

        auto IsDigit(char c) const -> bool
        {
            // from: https://en.cppreference.com/w/cpp/string/byte/isdigit
            return std::isdigit(static_cast<unsigned char>(c));
        }

        auto IsAlpha(char c) const -> bool
        {
            // from: https://en.cppreference.com/w/cpp/string/byte/isalpha
            return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
        }



        auto SkipWhitespace() -> void;


        auto Number() -> Token;
        auto IdentifierType() -> TokenType;
        auto Identifier() -> Token;
        auto String() -> Token;

        auto CheckKeyword(std::string_view s1, std::string_view s2, TokenType type) -> TokenType;
        
        // Return start + idx, current
        auto CurrentString(u32 idx = 0) -> std::string_view
        {
            return {start + idx, current};
        }
    
    private:
        std::string_view source;
        std::string_view::const_iterator start;
        std::string_view::const_iterator current;
        u32 current_line;
    };


} // namespace lox


#endif