#pragma once
#include <cassert>
#include <cstdint>
#include "./vec.hpp"


struct Slicer
{
    uint64_t size{0};
    uint64_t slice_count{1};
    uint64_t slice_size{0};
    uint64_t remaining{0};

    Slicer() = default;
    Slicer(size_t const size_, size_t const slice_count_)
        : size{size_}
    {
        setSliceCount(slice_count_);
    }

    void setSliceCount(size_t const slice_count_)
    {
        assert(slice_count_ > 0);
        slice_count = slice_count_;
        slice_size  = size / slice_count;
        remaining   = size - (slice_count * slice_size);
    }

    void setDimension(size_t const size_)
    {
        size = size_;
        setSliceCount(slice_count);
    }

    [[nodiscard]]
    Vec2u getSlice(size_t const slice_idx) const
    {
        assert(slice_idx < slice_count);
        auto const start = static_cast<uint32_t>(slice_size * slice_idx);
        auto const end   = static_cast<uint32_t>(slice_size * (slice_idx + 1));
        return {start, end};
    }

    [[nodiscard]]
    Vec2u getRemainingSlice() const
    {
        size_t const last_idx = slice_count * slice_size;
        return {
            static_cast<uint32_t>(last_idx),
            static_cast<uint32_t>(size)
        };
    }
};