// game.cpp
// CROQUEST prototype
// Defines core classes (Game, Scene, Entity, Platform, Skybox) in one file.

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

struct Rect {
    float x{0}, y{0}, w{0}, h{0};
};

struct Platform {
    float x{0}, y{0}, w{0}, h{0};
    int type{0}; // 0 = clip, 1 = clip-through-top-only, 2 = noclip
};

struct Entity {
    std::string type{"generic"};      // e.g., "player", "goomba"
    std::string skin{"color:#ffffff"}; // e.g., "color:#ff0000", "image:goomba.png"
    float x{0}, y{0};                  // position in world space
    float w{10}, h{10};                  // position in world space
    float dx{0}, dy{0};                // velocity
    float ddx{0}, ddy{0};                // passive acceleration
};

struct Scene {
    std::vector<Platform> platforms{};
    std::vector<Entity>  entities{};
};

class Game {
public:

    std::vector<Scene> scenes{};
    Game() = default;

    void parseWorld(const std::string&) {
        scenes.clear();

        Scene s;

        // ground platform spanning the bottom of the screen
        Platform ground;
        ground.x = 0;
        ground.y = 0;
        ground.w = 320;
        ground.h = 20;
        ground.type = 0;
        s.platforms.push_back(ground);

        // placeholder player entity
        Entity player;
        player.type = "player";
        player.skin = "color:#ff0000";
        player.x = 10;
        player.y = 20;
        player.ddy = -9.81f; // gravity
        s.entities.push_back(player);

        scenes.push_back(s);
    }

    const std::vector<Scene>& getScenes() const { return scenes; }

    // camera position, used for rendering
    float cameraX{0};
    float cameraY{0};

};