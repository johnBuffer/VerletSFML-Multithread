#pragma once
#include <iostream>
#include "collision_grid.hpp"
#include "physic_object.hpp"
#include "engine/common/utils.hpp"
#include "engine/common/index_vector.hpp"
#include "thread_pool/thread_pool.hpp"


struct PhysicSolver
{
    CIVector<PhysicObject> objects;
    CollisionGrid          grid;
    Vec2                   world_size;
    Vec2                   gravity = {0.0f, 20.0f};

    // Simulation solving pass count
    uint32_t        sub_steps;
    tp::ThreadPool& thread_pool;

    PhysicSolver(IVec2 size, tp::ThreadPool& tp)
        : grid{size.x, size.y}
        , world_size{to<float>(size.x), to<float>(size.y)}
        , sub_steps{8}
        , thread_pool{tp}
    {
        grid.clear();
    }

    // Checks if two atoms are colliding and if so create a new contact
    void solveContact(Vec2& pos_1, Vec2& pos_2)
    {
        if (&pos_1 == &pos_2) {
            return;
        }
        constexpr float response_coef = 1.0f;
        constexpr float eps           = 0.0001f;
        const Vec2 o2_o1  = pos_1 - pos_2;
        const float dist2 = o2_o1.x * o2_o1.x + o2_o1.y * o2_o1.y;
        if (dist2 < 1.0f && dist2 > eps) {
            const float dist          = sqrt(dist2);
            // Radius are all equal to 1.0f
            const float delta  = response_coef * 0.5f * (1.0f - dist);
            const Vec2 col_vec = (o2_o1 / dist) * delta;
            pos_1 += col_vec;
            pos_2 -= col_vec;
        }
    }

    void checkAtomCellCollisions(Vec2& pos_1, CollisionCell& c)
    {
        for (uint32_t i{0}; i < c.objects_count; ++i) {
            solveContact(pos_1, c.objects[i]);
        }
    }

    void processCell(CollisionCell& c, uint32_t index)
    {
        for (uint32_t i{0}; i < c.objects_count; ++i) {
            Vec2& position_1 = c.objects[i];
            checkAtomCellCollisions(position_1, grid.data[index - 1]);
            checkAtomCellCollisions(position_1, grid.data[index]);
            checkAtomCellCollisions(position_1, grid.data[index + 1]);
            checkAtomCellCollisions(position_1, grid.data[index + grid.height - 1]);
            checkAtomCellCollisions(position_1, grid.data[index + grid.height    ]);
            checkAtomCellCollisions(position_1, grid.data[index + grid.height + 1]);
            checkAtomCellCollisions(position_1, grid.data[index - grid.height - 1]);
            checkAtomCellCollisions(position_1, grid.data[index - grid.height    ]);
            checkAtomCellCollisions(position_1, grid.data[index - grid.height + 1]);
        }
    }

    void solveCollisionThreaded(uint32_t start, uint32_t end)
    {
        for (uint32_t idx{start}; idx < end; ++idx) {
            processCell(grid.data[idx], idx);
        }
    }

    // Find colliding atoms
    void solveCollisions()
    {
        // Multi-thread grid
        const uint32_t thread_count = thread_pool.m_thread_count;
        const uint32_t slice_count  = thread_count * 2;
        const uint32_t slice_size   = (grid.width / slice_count) * grid.height;
        const uint32_t last_cell    = (2 * (thread_count - 1) + 2) * slice_size;
        // Find collisions in two passes to avoid data races

        // First collision pass
        for (uint32_t i{0}; i < thread_count; ++i) {
            thread_pool.addTask([this, i, slice_size]{
                uint32_t const start{2 * i * slice_size};
                uint32_t const end  {start + slice_size};
                solveCollisionThreaded(start, end);
            });
        }
        // Eventually process rest if the world is not divisible by the thread count
        if (last_cell < grid.data.size()) {
            thread_pool.addTask([this, last_cell]{
                solveCollisionThreaded(last_cell, to<uint32_t>(grid.data.size()));
            });
        }
        thread_pool.waitForCompletion();
        // Second collision pass
        for (uint32_t i{0}; i < thread_count; ++i) {
            thread_pool.addTask([this, i, slice_size]{
                uint32_t const start{(2 * i + 1) * slice_size};
                uint32_t const end  {start + slice_size};
                solveCollisionThreaded(start, end);
            });
        }
        thread_pool.waitForCompletion();
    }

    // Add a new object to the solver
    uint64_t addObject(const PhysicObject& object)
    {
        return objects.push_back(object);
    }

    // Add a new object to the solver
    uint64_t createObject(Vec2 pos)
    {
        return objects.emplace_back(pos);
    }

    void update(float dt)
    {
        // Perform the sub steps
        const float sub_dt = dt / static_cast<float>(sub_steps);
        for (uint32_t i(sub_steps); i--;) {
            addObjectsToGrid();
            solveCollisions();
            updateObjects_multi(sub_dt);
        }
    }

    void addObjectsToGrid()
    {
        grid.clear();
        // Safety border to avoid adding object outside the grid
        for (PhysicObject& obj : objects.data) {
            if (obj.position.x > 1.0f && obj.position.x < world_size.x - 1.0f &&
                obj.position.y > 1.0f && obj.position.y < world_size.y - 1.0f) {
                obj.cell_idx = grid.addAtom(obj.position);
            }
        }
    }

    void updateObjects_multi(float dt)
    {
        thread_pool.dispatch(to<uint32_t>(objects.size()), [&](uint32_t start, uint32_t end){
            for (uint32_t i{start}; i < end; ++i) {
                PhysicObject& obj = objects.data[i];
                // Fetch updated position
                uint32_t const cell_idx = obj.cell_idx & 0xFFFFFFFF;
                uint32_t const data_idx = obj.cell_idx >> 32;
                if (data_idx != 255) {
                    obj.position = grid.data[cell_idx].objects[data_idx];
                }
                // Add gravity
                obj.acceleration += gravity;
                // Apply Verlet integration
                obj.update(dt);
                // Apply map borders collisions
                const float margin = 2.0f;
                if (obj.position.x > world_size.x - margin) {
                    obj.position.x = world_size.x - margin;
                } else if (obj.position.x < margin) {
                    obj.position.x = margin;
                }
                if (obj.position.y > world_size.y - margin) {
                    obj.position.y = world_size.y - margin;
                } else if (obj.position.y < margin) {
                    obj.position.y = margin;
                }
            }
        });
    }
};
