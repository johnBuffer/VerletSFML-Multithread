#pragma once
#include <SFML/Graphics/Color.hpp>
#include "utils.hpp"
#include "math.hpp"


struct ColorUtils
{
    template<typename T>
    static sf::Color createColor(T r, T g, T b)
    {
        return { to<uint8_t>(r), to<uint8_t>(g), to<uint8_t>(b) };
    }

    template<typename TVec3>
    static sf::Color createColor(TVec3 vec)
    {
        return { to<uint8_t>(vec.x), to<uint8_t>(vec.y), to<uint8_t>(vec.z) };
    }

    static sf::Color interpolate(sf::Color color_1, sf::Color color_2, float ratio)
    {
        return ColorUtils::createColor(
            to<float>(color_1.r) + ratio * to<float>(color_2.r - color_1.r),
            to<float>(color_1.g) + ratio * to<float>(color_2.g - color_1.g),
            to<float>(color_1.b) + ratio * to<float>(color_2.b - color_1.b)
        );
    }

    static sf::Color getRainbow(float t)
    {
        const float r = sin(t);
        const float g = sin(t + 0.33f * 2.0f * Math::PI);
        const float b = sin(t + 0.66f * 2.0f * Math::PI);
        return createColor(255 * r * r, 255 * g * g, 255 * b * b);
    }

};