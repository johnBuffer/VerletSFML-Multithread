#pragma once


struct SpringParticle
{
    float length_target  = 1.0f;
    float length_current = 1.0f;
    float speed     = 0.0f;
    float strength  = 100.0f;
    float damping   = 5.0f;

    SpringParticle() = default;

    SpringParticle(float target_length, float strength_, float damping_)
        : length_target(target_length)
        , length_current(target_length)
        , strength(strength_)
        , damping(damping_)
    {}

    void setX(float x)
    {
        length_current = x;
    }

    void addX(float dx)
    {
        length_current += dx;
    }

    void update(float dt)
    {
        speed     += ((length_target - length_current) * strength - speed * damping) * dt;
        length_current += speed * dt;
    }

    [[nodiscard]]
    float getLengthRatio() const
    {
        return length_current / length_target;
    }
};
