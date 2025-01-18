#include <iostream>

#include "engine/window_context_handler.hpp"
#include "engine/common/color_utils.hpp"
#include "engine/common/binary_io.hpp"

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
    const IVec2 world_size{150, 150};
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

    sf::Image image;
    image.loadFromFile("res/test.png");

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::L, [&](sfev::CstEv) {
        for (auto& obj : solver.objects) {
            IVec2 const pxl = static_cast<IVec2>(obj.position);
            sf::Color const color = image.getPixel(pxl.x, pxl.y);
            obj.color = color;
        }
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::W, [&](sfev::CstEv) {
        BinaryWriter writer{"colors.bin"};
        for (auto& obj : solver.objects) {
            writer.write(obj.color);
        }
    });

    std::vector<sf::Color> colors;

    BinaryReader color_reader{"colors.bin"};
    if (color_reader.isValid()) {
        for (uint32_t i{20000}; i--;) {
            colors.push_back(color_reader.read<sf::Color>());
        }
    }
    std::cout << "Loaded " << colors.size() << " colors" << std::endl;

    // Main loop
    const float dt = 1.0f / static_cast<float>(fps_cap);
    while (app.run()) {
        if (solver.objects.size() < 20000 && emit) {
            for (uint32_t i{20}; i--;) {
                const auto id = solver.createObject({2.0f, 10.0f + 1.1f * i});
                solver.objects[id].last_position.x -= 0.2f;
                if (colors.empty()) {
                    solver.objects[id].color = ColorUtils::getRainbow(id * 0.0001f);
                } else {
                    solver.objects[id].color = colors[id];
                }
            }
        }
        solver.update(dt);

        render_context.clear();
        renderer.render(render_context);
        render_context.display();
    }

    return 0;
}
