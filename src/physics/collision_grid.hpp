#pragma once
#include <cstdint>
#include "engine/common/vec.hpp"
#include "engine/common/grid.hpp"


struct CollisionCell
{
    static constexpr uint8_t cell_capacity = 4;
    static constexpr uint8_t max_cell_idx  = cell_capacity - 1;

    // Overlap workaround
	uint32_t objects_count              = 0;
    uint32_t objects[cell_capacity] = {};

	CollisionCell() = default;

	void addAtom(uint32_t id)
	{
        objects[objects_count] = id;
        objects_count += objects_count < max_cell_idx;
	}

	void clear()
	{
		objects_count = 0u;
	}

    void remove(uint32_t id)
    {
        for (uint32_t i{0}; i < objects_count; ++i) {
            if (objects[i] == id) {
                // Swap pop
                objects[i] = objects[objects_count - 1];
                --objects_count;
                return;
            }
        }

        //std::cout << "Problem" << std::endl;
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

	bool addAtom(uint32_t x, uint32_t y, uint32_t atom)
	{
		const uint32_t id = x * height + y;
		// Add to grid
		data[id].addAtom(atom);
		return true;
	}

	void clear()
	{
		for (auto& c : data) {
            c.objects_count = 0;
        }
	}
};
