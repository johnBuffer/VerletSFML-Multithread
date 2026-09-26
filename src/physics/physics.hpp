#pragma once
#include "collision_grid.hpp"
#include "physic_object.hpp"
#include "engine/common/utils.hpp"
#include "engine/common/index_vector.hpp"
#include "thread_pool/thread_pool.hpp"


struct PhysicSolver
{
    static constexpr float s_objects_radius = 8.0f;

    CIVector<PhysicObject> objects;
    CollisionGrid          grid;
    Vec2f                  world_size;
    Vec2f                  gravity{0.0f, 50.0f};

    // Simulation solving pass count
    uint32_t        sub_steps;
    tp::ThreadPool& thread_pool;

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
        constexpr float response_coef = 0.5f;
        constexpr float eps           = 0.0001f;
        PhysicObject& obj_1 = objects.data[atom_1_idx];
        PhysicObject& obj_2 = objects.data[atom_2_idx];
        const Vec2f o2_o1  = obj_1.position - obj_2.position;
        const float dist2 = o2_o1.x * o2_o1.x + o2_o1.y * o2_o1.y;
        float constexpr min_dist = 2.0f * s_objects_radius;
        if (dist2 < (min_dist * min_dist) && dist2 > eps) {
            const float dist          = sqrt(dist2);
            // Radius are all equal to 1.0f
            const float delta  = response_coef * 0.5f * (min_dist - dist);
            const Vec2f col_vec = (o2_o1 / dist) * delta;
            obj_1.position += col_vec;
            obj_2.position -= col_vec;
        }
    }

    // Find colliding atoms
    void solveCollisions()
    {
        size_t const objects_count = objects.data.size();
        for (size_t i{0}; i < objects_count; ++i) {
            for (size_t k{0}; k < objects_count; ++k) {
                solveContact(i, k);
            }
        }
    }

    // Add a new object to the solver
    uint64_t addObject(const PhysicObject& object)
    {
        return objects.push_back(object);
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
            solveCollisions();
            updateObjectsMulti(sub_dt);
        }
    }

    void updateObjectsMulti(float const dt) {
        thread_pool.dispatch(static_cast<uint32_t>(objects.size()), [&](uint32_t const start, uint32_t const end){
            for (uint32_t i{start}; i < end; ++i) {
                PhysicObject& obj = objects.data[i];
                // Apply PBD integration
                obj.update(dt, gravity);
                // Apply boundaries constraints
                constexpr float margin = 2.0f;
                obj.position.x = std::clamp(obj.position.x, s_objects_radius, world_size.x - s_objects_radius);
                obj.position.y = std::clamp(obj.position.y, s_objects_radius, world_size.y - s_objects_radius);
            }
        });
    }
};
