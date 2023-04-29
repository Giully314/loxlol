/*
c++/lox/common.hpp

PURPOSE:
    define standard types across the project.
*/

#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <vector>
#include <stack>

namespace lox
{
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;

    using i32 = std::int32_t;    

    // A non owned res must be non nullable and must not be deleted. It is assumed to be always safe to access 
    // the pointer because the life time is controlled.
    template <typename T>
    using non_owned_res = T*;

    template <typename T>
    using non_nullable_res = T*;

    // template <typename T>
    // using Stack = std::stack<T, std::vector<T>>;

    // For debug purpose, we need to visualize the stack. It's impossible to traverse std::stack without
    // performing copy or other stuff, so we use std::vector.
    template <typename T>
    using Stack = std::vector<T>;
} // namespace lox


#endif