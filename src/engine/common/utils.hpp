#pragma once
#include "index_vector.hpp"
#include <sstream>


template<typename U, typename T>
U to(const T& v)
{
    return static_cast<U>(v);
}


template<typename T>
using CIVector = civ::Vector<T>;


template<typename T>
T sign(T v)
{
    return v < 0.0f ? -1.0f : 1.0f;
}


template<typename T>
static std::string toString(T value)
{
    std::stringstream sx;
    sx << value;
    return sx.str();
}

