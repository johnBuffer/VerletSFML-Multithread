#pragma once
#include "collision_grid.hpp"
#include "engine/common/utils.hpp"
#include "engine/common/math.hpp"


struct PhysicObject
{
    // Verlet
    Vec2f position      = {0.0f, 0.0f};
    Vec2f last_position = {0.0f, 0.0f};
    int32_t grid_cell_idx = 0;
    sf::Color color;

    PhysicObject() = default;

    explicit
    PhysicObject(Vec2f position_)
        : position(position_)
        , last_position(position_)
    {}

    void setPosition(Vec2f const pos)
    {
        position      = pos;
        last_position = pos;
    }

    void update(float const dt, Vec2f const gravity)
    {
        const Vec2f last_update_move = position - last_position;
        //const Vec2 new_position = position + last_update_move + (gravity - last_update_move * 80.0f) * (dt * dt);
        const Vec2f new_position = position + last_update_move + gravity * (dt * dt);
        last_position           = position;
        position                = new_position;
    }

    void stop()
    {
        last_position = position;
    }

    void slowdown(float const ratio)
    {
        last_position = last_position + ratio * (position - last_position);
    }

    [[nodiscard]]
    float getSpeed() const
    {
        return MathVec2::length(position - last_position);
    }

    [[nodiscard]]
    Vec2f getVelocity() const
    {
        return position - last_position;
    }

    void addVelocity(Vec2f const v)
    {
        last_position -= v;
    }

    void setPositionSameSpeed(Vec2f const new_position)
    {
        const Vec2f to_last = last_position - position;
        position           = new_position;
        last_position      = position + to_last;
    }

    void move(Vec2f v)
    {
        position += v;
    }
};
