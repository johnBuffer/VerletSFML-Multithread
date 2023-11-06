#pragma once
#include <cstdint>
#include "engine/common/vec.hpp"
#include "engine/common/grid.hpp"
#include "engine/common/utils.hpp"


struct CollisionCell
{
    static constexpr uint8_t cell_capacity = 4;
    static constexpr uint8_t max_cell_idx  = cell_capacity - 1;

    // Overlap workaround
	uint32_t objects_count          = 0;
    Vec2     objects[cell_capacity] = {};

	CollisionCell() = default;

	uint32_t addAtom(Vec2 position)
	{
        if (objects_count == cell_capacity) {
            return 255;
        }
        objects[objects_count] = position;
        return objects_count++;
	}

	void clear()
	{
		objects_count = 0u;
	}
};

struct CollisionGrid : public Grid<CollisionCell>
{
	CollisionGrid()
		: Grid<CollisionCell>()
	{}

	CollisionGrid(int32_t width, int32_t height)
		: Grid<CollisionCell>(width, height)
	{}

    uint32_t getIndex(Vec2 pos)
    {
        return to<int32_t>(pos.x) * height + to<int32_t>(pos.y);
    }

	uint64_t addAtom(Vec2 position)
	{
		// Add to grid
        uint64_t const cell_idx = getIndex(position);
        uint64_t const data_idx = data[cell_idx].addAtom(position);
        return cell_idx | (data_idx << 32);
	}

    Vec2 getPosition(Vec2 old, uint32_t idx)
    {
        return data[getIndex(old)].objects[idx];
    }

	void clear()
	{
		for (auto& c : data) {
            c.objects_count = 0;
        }
	}
};
