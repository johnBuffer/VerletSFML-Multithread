#pragma once
#include <SFML/Graphics.hpp>
#include "collision_grid.hpp"
#include "engine/common/utils.hpp"
#include "engine/common/math.hpp"


struct PhysicObject
{
    // Verlet
    Vec2 position           = {0.0f, 0.0f};
    Vec2 position_candidate = {0.0f, 0.0f};
    Vec2 velocity           = {0.0f, 0.0f};
    Vec2 acceleration       = {0.0f, 0.0f};
    sf::Color color{};

    PhysicObject() = default;

    explicit
    PhysicObject(Vec2 const position_)
        : position(position_)
        , position_candidate(position_)
    {}

    void setPosition(Vec2 const new_position)
    {
        position           = new_position;
        position_candidate = new_position;
    }

    void updateCandidatePosition(float const dt)
    {
        position_candidate = position + velocity * dt;
    }

    void update(float const dt)
    {
        velocity = (position_candidate - position) / dt;
        position = position_candidate;
        acceleration = {0.0f, 0.0f};
    }

    void stop()
    {
        velocity = {0.0f, 0.0f};
    }

    void slowdown(float const ratio)
    {
        velocity *= (1.0f - ratio);
    }

    [[nodiscard]]
    float getSpeed() const
    {
        return MathVec2::length(velocity);
    }

    [[nodiscard]]
    Vec2 getVelocity() const
    {
        return velocity;
    }

    void addVelocity(Vec2 const v)
    {
        velocity += v;
    }

    void setPositionSameSpeed(Vec2 const new_position)
    {
        const Vec2 to_last = position_candidate - position;
        position           = new_position;
        position_candidate = position + to_last;
    }

    void move(Vec2 const v)
    {
        position += v;
    }
};
