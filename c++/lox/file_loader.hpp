/*
c++/lox/fileloader.hpp

PURPOSE: load the lox files.

CLASSES:
    FileLoader: 
        Preprocess the file and load them into a string.

DESCRIPTION:
    The FileLoader does a similar job as the c preprocessor. Resolve the includes with a copy-paste approach
    and load the result into a string which is passed to the compiler. It's important that the file loader
    outlives the compiler because the compiler has a pointer to the source string of the file loader.
*/


#ifndef FILE_LOADER_HPP
#define FILE_LOADER_HPP

#include <string_view>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>

namespace lox
{
    class FileLoader
    {
    public:
        FileLoader(std::string_view filename)
        {
            std::ifstream file{filename.data()};
            if (!file)
            {
                throw std::runtime_error{"No file"};
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            source = buffer.str();
        }

        auto GetSource() const -> std::string_view
        {
            return source;
        }

    private:
        std::string source;
    };
    
} // namespace lox


#endif