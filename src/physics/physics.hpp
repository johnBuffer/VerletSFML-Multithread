#pragma once
#include "engine/common/index_vector.hpp"
#include "engine/common/slicer.hpp"
#include "engine/common/utils.hpp"
#include "thread_pool/thread_pool.hpp"

#include "collision_grid.hpp"
#include "physic_object.hpp"

struct PhysicSolver
{
    CIVector<PhysicObject> objects;
    CollisionGrid          grid;
    Vec2f                  world_size;
    Vec2f                  gravity{0.0f, 20.0f};
    uint32_t               sub_steps;
    tp::ThreadPool&        thread_pool;

    PhysicSolver(Vec2i const size, tp::ThreadPool& tp)
        : grid{size.x, size.y}
        , world_size{to<float>(size.x), to<float>(size.y)}
        , sub_steps{8}
        , thread_pool{tp}
    {
        grid.clear();
    }

    // Checks if two atoms are colliding and if so create a new contact
    void solveContact(uint32_t const atom_1_idx, uint32_t const atom_2_idx)
    {
        constexpr float response_coef = 0.95f;
        constexpr float eps           = 0.0001f;
        PhysicObject&   obj_1         = objects.data[atom_1_idx];
        PhysicObject&   obj_2         = objects.data[atom_2_idx];
        const Vec2f     o2_o1         = obj_1.position - obj_2.position;
        const float     dist2         = o2_o1.x * o2_o1.x + o2_o1.y * o2_o1.y;
        if (dist2 < 1.0f && dist2 > eps) {
            const float dist = sqrt(dist2);
            // Radius are all equal to 1.0f
            const float delta   = response_coef * 0.5f * (1.0f - dist);
            const Vec2f col_vec = (o2_o1 / dist) * delta;
            obj_1.position += col_vec;
            obj_2.position -= col_vec;
        }
    }

    void checkAtomCellCollisions(uint32_t const atom_idx, const CollisionCell& c)
    {
        for (uint32_t i{0}; i < c.objects_count; ++i) {
            solveContact(atom_idx, c.objects[i]);
        }
    }

    void processCell(const CollisionCell& c, uint32_t const index)
    {
        for (uint32_t i{0}; i < c.objects_count; ++i) {
            const uint32_t atom_idx = c.objects[i];
            checkAtomCellCollisions(atom_idx, grid.data[index]);
            checkAtomCellCollisions(atom_idx, grid.data[index + 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index + grid.height - 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index + grid.height]);
            checkAtomCellCollisions(atom_idx, grid.data[index + grid.height + 1]);
            // Double checks
            checkAtomCellCollisions(atom_idx, grid.data[index - 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index - grid.height - 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index - grid.height]);
            checkAtomCellCollisions(atom_idx, grid.data[index - grid.height + 1]);
        }
    }

    void solveColumnCollisionThread(Vec2u const slice)
    {
        thread_pool.addTask([this, slice] {
            auto const height = static_cast<uint32_t>(grid.height);
            for (uint32_t x{slice.x}; x < slice.y; ++x) {
                uint32_t const column = x * height;
                for (uint32_t y{0}; y < height; ++y) {
                    uint32_t const idx = column + y;
                    processCell(grid.data[idx], idx);
                }
            }
        });
    }

    // Find colliding atoms
    void solveCollisions()
    {
        size_t const thread_count{thread_pool.m_thread_count};
        Slicer const slicer{static_cast<size_t>(grid.width), 2 * thread_count};
        // First collision pass
        for (uint32_t i{0}; i < thread_count; ++i) {
            solveColumnCollisionThread(slicer.getSlice(2 * i));
        }
        if (slicer.remaining) {
            solveColumnCollisionThread(slicer.getRemainingSlice());
        }
        thread_pool.waitForCompletion();
        // Second collision pass
        for (uint32_t i{0}; i < thread_count; ++i) {
            solveColumnCollisionThread(slicer.getSlice(2 * i + 1));
        }
        thread_pool.waitForCompletion();
    }

    // Add a new object to the solver
    uint64_t createObject(Vec2f pos)
    {
        return objects.emplace_back(pos);
    }

    void update(float const dt)
    {
        // Perform the sub steps
        const float sub_dt = dt / static_cast<float>(sub_steps);
        for (uint32_t i(sub_steps); i--;) {
            updateGrid();
            solveCollisions();
            updateObjectsMulti(sub_dt);
        }
    }

    void updateGrid()
    {
        int32_t i{0};
        for (PhysicObject& obj : objects.data) {
            int32_t const idx = grid.getIndex(obj.position);
            if (idx != obj.grid_cell_idx) {
                grid.moveAtom(i, obj.grid_cell_idx, idx);
                obj.grid_cell_idx = idx;
            }
            ++i;
        }
    }

    void updateObjectsMulti(float const dt)
    {
        thread_pool.dispatch(static_cast<uint32_t>(objects.size()), [&](uint32_t const start, uint32_t const end) {
            for (uint32_t i{start}; i < end; ++i) {
                PhysicObject& obj = objects.data[i];
                // Apply PBD integration
                obj.update(dt, gravity);
                // Apply boundaries constraints
                constexpr float margin = 2.0f;
                obj.position.x         = std::clamp(obj.position.x, margin, world_size.x - margin);
                obj.position.y         = std::clamp(obj.position.y, margin, world_size.y - margin);
            }
        });
    }
};
