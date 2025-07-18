#pragma once
#include "index_vector.hpp"
#include <sstream>

template<typename T>
using CIVector = civ::Vector<T>;


template<typename T>
T sign(T v)
{
    return v < 0.0f ? -1.0f : 1.0f;
}


template<typename TVec>
TVec normal(TVec const& vec)
{
    return {-vec.y, vec.x};
}

template<typename TVec>
float dot(TVec const& a, TVec const& b)
{
    return a.x * b.x + a.y * b.y;
}


template<typename T>
static std::string toString(T value)
{
    std::stringstream sx;
    sx << value;
    return sx.str();
}


template<typename T>
sf::Vector2f toVector2f(sf::Vector2<T> v)
{
    return {static_cast<float>(v.x), static_cast<float>(v.y)};
}
