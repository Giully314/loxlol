/*
c++/lox/scanner.cpp
*/

#include "scanner.hpp"
#include "token.hpp"



namespace lox
{
    auto Scanner::Number() -> Token
    {
        while (IsDigit(Peek())) Advance();

        // Look for a fractional part.
        if (Peek() == '.' && IsDigit(PeekNext())) 
        {
            // Consume the ".".
            Advance();
            while (IsDigit(Peek())) Advance();
        }

        return MakeToken(TokenType::Number);
    }

    auto CheckKeyword(std::string_view s1, std::string_view s2, TokenType type) -> TokenType
    {
        if (s1 == s2)
        {
            return type;
        }
        
        return TokenType::Identifier;
    }

    auto Scanner::IdentifierType() -> TokenType
    {
        switch (*start) 
        {
            case 'a': return CheckKeyword(CurrentString(1), "nd", TokenType::And);
            case 'c': return CheckKeyword(CurrentString(1), "lass", TokenType::Class);
            case 'e': return CheckKeyword(CurrentString(1), "lse", TokenType::Else);
            case 'f':
                if (current - start > 1) 
                {
                    switch (start[1]) 
                    {
                        case 'a': return CheckKeyword(CurrentString(2), "lse",  TokenType::False);
                        case 'o': return CheckKeyword(CurrentString(2), "r",    TokenType::For);
                        case 'u': return CheckKeyword(CurrentString(2), "n",    TokenType::Fun);
                    }
                }
                break;
            case 'i': return CheckKeyword(CurrentString(1), "f",     TokenType::If);
            case 'n': return CheckKeyword(CurrentString(1), "il",    TokenType::Nil);
            case 'o': return CheckKeyword(CurrentString(1), "r",     TokenType::Or);
            case 'p': return CheckKeyword(CurrentString(1), "rint",  TokenType::Print);
            case 'r': return CheckKeyword(CurrentString(1), "eturn", TokenType::Return);
            case 's': return CheckKeyword(CurrentString(1), "uper",  TokenType::Super);
            case 't':
                if (current - start > 1) 
                {
                    switch (start[1]) 
                    {
                        case 'h': return CheckKeyword(CurrentString(2), "is", TokenType::This);
                        case 'r': return CheckKeyword(CurrentString(2), "ue", TokenType::True);
                    }
                }
                break;
            case 'v': return CheckKeyword(CurrentString(1), "ar",   TokenType::Var);
            case 'w': return CheckKeyword(CurrentString(1), "hile", TokenType::While);
        }
        return TokenType::Identifier;
    }

    auto Scanner::Identifier() -> Token
    {
        while (IsAlpha(Peek()) || IsDigit(Peek())) 
        {
            Advance();
        }
        return MakeToken(IdentifierType());
    }


    auto Scanner::String() -> Token
    {
        while (Peek() != '"' && !IsAtEnd()) 
        {
            if (Peek() == '\n') ++current_line;
            Advance();
        }

        if (IsAtEnd()) return ErrorToken("Unterminated string.");

        // The closing quote.
        Advance();
        return MakeToken(TokenType::String);
    }


    auto Scanner::SkipWhitespace() -> void
    {
        while (true) 
        {
            char c = Peek();
            switch (c) {
                case ' ':
                case '\r':
                case '\t':
                    Advance();
                    break;
                case '\n':
                    ++current_line;
                    Advance();
                    break;
                case '/':
                    if (PeekNext() == '/') 
                    {
                        // A comment goes until the end of the line.
                        while (Peek() != '\n' && !IsAtEnd()) Advance();
                    } 
                    else 
                    {
                        return;
                    }
                    break;
                default:
                    return;
            }
        }
    }


    auto Scanner::ScanToken() -> Token
    {
        SkipWhitespace();
        start = current;

        if (IsAtEnd())
        {
            return MakeToken(TokenType::Eof);
        }
        
        char c = Advance();

        if (IsDigit(c)) return Number();
        if (IsAlpha(c)) return Identifier();

        switch (c) 
        {
            case '(': return MakeToken(TokenType::LeftParen);
            case ')': return MakeToken(TokenType::RightParen);
            case '{': return MakeToken(TokenType::LeftBrace);
            case '}': return MakeToken(TokenType::RightBrace);
            case ';': return MakeToken(TokenType::Semicolon);
            case ',': return MakeToken(TokenType::Comma);
            case '.': return MakeToken(TokenType::Dot);
            case '-': return MakeToken(TokenType::Minus);
            case '+': return MakeToken(TokenType::Plus);
            case '/': return MakeToken(TokenType::Slash);
            case '*': return MakeToken(TokenType::Star);

            case '!': return MakeToken(Match('=') ? TokenType::BangEqual : TokenType::Bang);
            case '=': return MakeToken(Match('=') ? TokenType::EqualEqual : TokenType::Equal);
            case '<': return MakeToken(Match('=') ? TokenType::LessEqual : TokenType::Less);
            case '>': return MakeToken(Match('=') ? TokenType::GreaterEqual : TokenType::Greater);
        
            case '"': return String();
        }

        return ErrorToken("Unexpected character.");
    }

} // namespace lox