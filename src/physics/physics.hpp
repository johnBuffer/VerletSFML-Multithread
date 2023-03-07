#pragma once
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
        : grid(size.x, size.y)
        , sub_steps(8)
        , world_size(to<float>(size.x), to<float>(size.y))
        , thread_pool{tp}
    {
        grid.clear();
    }

    // Checks if two atoms are colliding and if so create a new contact
    void solveContact(uint32_t atom_1_idx, uint32_t atom_2_idx)
    {
        constexpr float response_coef = 1.0f;
        constexpr float eps           = 0.0001f;
        PhysicObject& obj_1 = objects.data[atom_1_idx];
        PhysicObject& obj_2 = objects.data[atom_2_idx];
        const Vec2 o2_o1  = obj_1.position - obj_2.position;
        const float dist2 = o2_o1.x * o2_o1.x + o2_o1.y * o2_o1.y;
        if (dist2 < 1.0f && dist2 > eps) {
            const float dist          = sqrt(dist2);
            // Radius are all equal to 1.0f
            const float delta  = response_coef * 0.5f * (1.0f - dist);
            const Vec2 col_vec = (o2_o1 / dist) * delta;
            obj_1.position += col_vec;
            obj_2.position -= col_vec;
        }
    }

    void checkAtomCellCollisions(uint32_t atom_idx, const CollisionCell& c)
    {
        for (uint32_t i{0}; i < c.objects_count; ++i) {
            solveContact(atom_idx, c.objects[i]);
        }
    }

    void processCell(const CollisionCell& c, uint32_t index)
    {
        for (uint32_t i{0}; i < c.objects_count; ++i) {
            const uint32_t atom_idx = c.objects[i];
            checkAtomCellCollisions(atom_idx, grid.data[index - 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index]);
            checkAtomCellCollisions(atom_idx, grid.data[index + 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index + grid.height - 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index + grid.height    ]);
            checkAtomCellCollisions(atom_idx, grid.data[index + grid.height + 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index - grid.height - 1]);
            checkAtomCellCollisions(atom_idx, grid.data[index - grid.height    ]);
            checkAtomCellCollisions(atom_idx, grid.data[index - grid.height + 1]);
        }
    }

    void solveCollisionThreaded(uint32_t i, uint32_t slice_size)
    {
        const uint32_t start = i * slice_size;
        const uint32_t end   = (i + 1) * slice_size;
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

        for (uint32_t i{0}; i < thread_count; ++i) {
            thread_pool.addTask([this, i, slice_size]{
                solveCollisionThreaded(2 * i, slice_size);
            });
        }
        thread_pool.waitForCompletion();

        for (uint32_t i{0}; i < thread_count; ++i) {
            thread_pool.addTask([this, i, slice_size]{
                solveCollisionThreaded(2 * i + 1, slice_size);
            });
        }
        thread_pool.waitForCompletion();
    }

    // Add a new object to the solver
    uint64_t addObject(const PhysicObject& object)
    {
        const uint64_t object_id = objects.push_back(object);
        /*PhysicObject& new_object = objects[object_id];*/
        return object_id;
    }

    // Add a new object to the solver
    uint64_t createObject(Vec2 pos)
    {
        const civ::ID object_id = objects.emplace_back(pos);
        /*PhysicObject* new_object = &objects[object_id];*/
        return object_id;
    }

    void update(float dt)
    {
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

        uint32_t i{0};
        for (const PhysicObject& obj : objects.data) {
            if (obj.position.x > 1.0f && obj.position.x < world_size.x - 1.0f &&
                obj.position.y > 1.0f && obj.position.y < world_size.y - 1.0f) {
                grid.addAtom(to<int32_t>(obj.position.x), to<int32_t>(obj.position.y), i);
            }
            ++i;
        }
    }

    void updateObjects_multi(float dt)
    {
        thread_pool.dispatch(to<uint32_t>(objects.size()), [&](uint32_t start, uint32_t end){
            for (uint32_t i{start}; i < end; ++i) {
                PhysicObject& obj = objects.data[i];
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
