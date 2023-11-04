#include <iostream>

#include "engine/window_context_handler.hpp"
#include "engine/common/color_utils.hpp"

#include "physics/physics.hpp"
#include "thread_pool/thread_pool.hpp"
#include "renderer/renderer.hpp"


int main()
{
    const uint32_t window_width  = 1920;
    const uint32_t window_height = 1080;
    WindowContextHandler app("Verlet-MultiThread", sf::Vector2u(window_width, window_height), sf::Style::Default);
    RenderContext& render_context = app.getRenderContext();
    // Initialize solver and renderer

    tp::ThreadPool thread_pool(10);
    const IVec2 world_size{300, 300};
    PhysicSolver solver{world_size, thread_pool};
    Renderer renderer(solver, thread_pool);

    const float margin = 20.0f;
    const auto  zoom   = static_cast<float>(window_height - margin) / static_cast<float>(world_size.y);
    render_context.setZoom(zoom);
    render_context.setFocus({world_size.x * 0.5f, world_size.y * 0.5f});

    bool emit = true;
    app.getEventManager().addKeyPressedCallback(sf::Keyboard::Space, [&](sfev::CstEv) {
        emit = !emit;
    });

    constexpr uint32_t fps_cap = 60;
    int32_t target_fps = fps_cap;
    app.getEventManager().addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv) {
        target_fps = target_fps ? 0 : fps_cap;
        app.setFramerateLimit(target_fps);
    });

    // Main loop
    const float dt = 1.0f / static_cast<float>(fps_cap);
    while (app.run()) {
        if (solver.objects.size() < 80000 && emit) {
            for (uint32_t i{20}; i--;) {
                const auto id = solver.createObject({2.0f, 10.0f + 1.1f * i});
                solver.objects[id].last_position.x -= 0.2f;
                solver.objects[id].color = ColorUtils::getRainbow(id * 0.0001f);
            }
        }

        solver.update(dt);

        render_context.clear();
        renderer.render(render_context);
        render_context.display();
    }

    return 0;
}
