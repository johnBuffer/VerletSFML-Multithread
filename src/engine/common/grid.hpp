#pragma once
#include <array>
#include <vector>


template<typename T>
struct Grid
{
    int32_t        width, height;
    std::vector<T> data;

    Grid()
        : width(0)
        , height(0)
    {
    }

    Grid(int32_t const width_, int32_t const height_)
        : width(width_)
        , height(height_)
    {
        data.resize(width * height);
    }

    [[nodiscard]]
    int32_t getIndex(int32_t const x, int32_t const y) const
    {
        return x * height + y;
    }

    [[nodiscard]]
    int32_t getIndex(Vec2f const pos) const
    {
        return getIndex(pos.x, pos.y);
    }
};
