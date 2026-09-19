#include "renderer.hpp"


Renderer::Renderer(PhysicSolver& solver_, tp::ThreadPool& tp)
    : solver{solver_}
    , world_va{sf::PrimitiveType::Triangles, 6}
    , objects_va{sf::PrimitiveType::Triangles}
    , object_texture{"res/circle.png"}
    , thread_pool{tp}
{
    initializeWorldVA();

    [[maybe_unused]] const bool generated_mipmap = object_texture.generateMipmap();
    object_texture.setSmooth(true);
}

void Renderer::render(RenderContext& context)
{
    renderHUD(context);
    context.draw(world_va);

    sf::RenderStates states;
    states.texture = &object_texture;
    context.draw(world_va, states);
    // Particles
    updateParticlesVA();
    context.draw(objects_va, states);
}

void Renderer::initializeWorldVA()
{
    const sf::Vector2f p0{0.0f, 0.0f};
    const sf::Vector2f p1{solver.world_size.x, 0.0f};
    const sf::Vector2f p2{solver.world_size.x, solver.world_size.y};
    const sf::Vector2f p3{0.0f, solver.world_size.y};

    world_va[0].position = p0;
    world_va[1].position = p1;
    world_va[2].position = p2;
    world_va[3].position = p0;
    world_va[4].position = p2;
    world_va[5].position = p3;

    const uint8_t level = 50;
    const sf::Color background_color{level, level, level};
    for (std::size_t i = 0; i < 6; ++i) {
        world_va[i].color = background_color;
    }
}

void Renderer::updateParticlesVA()
{
    objects_va.resize(solver.objects.size() * 6);

    const float texture_size = 1024.0f;
    const float radius       = 0.5f;
    thread_pool.dispatch(to<uint32_t>(solver.objects.size()), [&](uint32_t start, uint32_t end) {
        for (uint32_t i{start}; i < end; ++i) {
            const PhysicObject& object = solver.objects.data[i];
            const uint32_t idx = i * 6;
            const Vec2 p0 = object.position + Vec2{-radius, -radius};
            const Vec2 p1 = object.position + Vec2{ radius, -radius};
            const Vec2 p2 = object.position + Vec2{ radius,  radius};
            const Vec2 p3 = object.position + Vec2{-radius,  radius};
            const sf::Vector2f t0{0.0f, 0.0f};
            const sf::Vector2f t1{texture_size, 0.0f};
            const sf::Vector2f t2{texture_size, texture_size};
            const sf::Vector2f t3{0.0f, texture_size};
            const sf::Color color = object.color;

            objects_va[idx + 0].position = p0;
            objects_va[idx + 1].position = p1;
            objects_va[idx + 2].position = p2;
            objects_va[idx + 3].position = p0;
            objects_va[idx + 4].position = p2;
            objects_va[idx + 5].position = p3;

            objects_va[idx + 0].texCoords = t0;
            objects_va[idx + 1].texCoords = t1;
            objects_va[idx + 2].texCoords = t2;
            objects_va[idx + 3].texCoords = t0;
            objects_va[idx + 4].texCoords = t2;
            objects_va[idx + 5].texCoords = t3;

            objects_va[idx + 0].color = color;
            objects_va[idx + 1].color = color;
            objects_va[idx + 2].color = color;
            objects_va[idx + 3].color = color;
            objects_va[idx + 4].color = color;
            objects_va[idx + 5].color = color;
        }
    });
}

void Renderer::renderHUD(RenderContext&)
{
    // HUD
    /*const float margin    = 20.0f;
    float       current_y = margin;
    text_time.setString("Simulation time: " + toString(phys_time.get()) + "ms");
    text_time.setPosition({margin, current_y});
    current_y += text_time.getBounds().y + 0.5f * margin;
    context.renderToHUD(text_time, RenderContext::Mode::Normal);

    text_objects.setString("Objects: " + toString(simulation.solver.objects.size()));
    text_objects.setPosition({margin, current_y});
    current_y += text_objects.getBounds().y + 0.5f * margin;
    context.renderToHUD(text_objects, RenderContext::Mode::Normal);*/
}
