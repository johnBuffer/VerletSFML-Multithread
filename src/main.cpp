#include <iomanip>
#include <sstream>

#include "engine/window_context_handler.hpp"
#include "engine/common/average_timer.hpp"
#include "engine/common/color_utils.hpp"

#include "physics/physics.hpp"
#include "thread_pool/thread_pool.hpp"
#include "renderer/renderer.hpp"


int main()
{
    const uint32_t window_width  = 1600;
    const uint32_t window_height = 900;
    WindowContextHandler app("Verlet-MultiThread", sf::Vector2u(window_width, window_height), sf::Style::Default);
    RenderContext& render_context = app.getRenderContext();
    // Initialize solver and renderer

    tp::ThreadPool thread_pool(15);
    const Vec2i world_size{300, 300};
    PhysicSolver solver{world_size, thread_pool};
    Renderer renderer(solver, thread_pool);

    const float margin = 20.0f;
    const auto  zoom   = static_cast<float>(window_height - margin) / static_cast<float>(world_size.y);
    render_context.setZoom(zoom);
    render_context.setFocus({world_size.x * 0.5f, world_size.y * 0.5f});

    std::vector<sf::Color> colors;

    bool emit = true;
    app.getEventManager().addKeyPressedCallback(sf::Keyboard::Key::Space, [&](sfev::CstEv) {
        emit = !emit;
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::Key::L, [&](sfev::CstEv) {
        sf::Image image;
        image.loadFromFile("res/chicken.png");
        Vec2f const image_size_f{image.getSize()};
        Vec2f const world_size_f{world_size};
        Vec2f const scale = image_size_f.componentWiseDiv(world_size_f);
        colors.clear();
        colors.reserve(solver.objects.size());
        for (auto& o : solver.objects) {
            Vec2f const pxl_f = o.position.componentWiseMul(scale);
            sf::Vector2u const pxl{
                static_cast<uint32_t>(std::clamp(pxl_f.x, 0.0f, image_size_f.x)),
                static_cast<uint32_t>(std::clamp(pxl_f.y, 0.0f, image_size_f.y))
            };
            o.color = image.getPixel(pxl);
            colors.push_back(o.color);
        }
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::Key::R, [&](sfev::CstEv) {
        solver.objects.clear();
        solver.grid.clear();
    });

    constexpr uint32_t fps_cap = 60;
    int32_t target_fps = fps_cap;
    app.getEventManager().addKeyPressedCallback(sf::Keyboard::Key::S, [&](sfev::CstEv) {
        target_fps = target_fps ? 0 : fps_cap;
        app.setFramerateLimit(target_fps);
    });

    // Main loop
    const float dt = 1.0f / static_cast<float>(fps_cap);
    AverageTimer physics_timer{fps_cap};
    AverageTimer render_timer{fps_cap};

    const sf::Font font{"res/roboto_regular.ttf"};
    sf::Text hud_text{font, "", 20};
    hud_text.setFillColor(sf::Color::White);
    hud_text.setOutlineColor(sf::Color::Black);
    hud_text.setOutlineThickness(1.0f);
    hud_text.setPosition({20.0f, 20.0f});

    while (app.run()) {
        if (solver.objects.size() < 100000 && emit) {
            for (uint32_t i{20}; i--;) {
                const auto id = solver.createObject({2.0f, 10.0f + 1.1f * i});
                solver.objects[id].last_position.x -= 0.13f;
                uint64_t const idx = solver.objects.size();
                if (idx < colors.size()) {
                    solver.objects[id].color = colors[idx - 1];
                } else {
                    solver.objects[id].color = ColorUtils::getRainbow(id * 0.0001f);
                }
            }
        }

        physics_timer.start();
        solver.update(dt);
        physics_timer.stop();

        render_timer.start();
        render_context.clear();
        renderer.render(render_context);
        render_timer.stop();

        const float physics_ms = physics_timer.get();
        const float render_ms  = render_timer.get();
        const float frame_ms   = physics_ms + render_ms;
        std::ostringstream hud;
        hud << std::fixed << std::setprecision(2)
            << "Physics: " << physics_ms << " ms\n"
            << "Render:  " << render_ms << " ms\n"
            << "Frame:   " << frame_ms << " ms\n"
            << "Objects: " << solver.objects.size();
        hud_text.setString(hud.str());
        render_context.drawDirect(hud_text);

        render_context.display();
    }

    return 0;
}
