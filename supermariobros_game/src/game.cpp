// game.cpp
// CROQUEST prototype
// Defines core classes (Game, Scene, Entity, Platform, Skybox) in one file.

#include <Arduino.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

struct Rect
{
    float x{0}, y{0}, w{0}, h{0};
};

struct Platform
{
    float x{0}, y{0}, w{0}, h{0};
    int type{0};     // 0 = clip, 1 = clip-through-top-only, 2 = noclip
    float bouncy{0}; // 0 = no bounce, 1 = bounce
};

struct Entity
{
    std::string type{"generic"};       // e.g., "player", "goomba"
    std::string skin{"color:#ffffff"}; // e.g., "color:#ff0000", "image:goomba.png"
    float x{0}, y{0};                  // position in world space
    float w{10}, h{10};                // position in world space
    float dx{0}, dy{0};                // velocity
    float ddx{0}, ddy{0};              // passive acceleration
};

class Flagpole : public Entity
{
    bool isUp = true; // private state

public:
    Flagpole()
    {
        type = "flagpole";
        skin = "color:#ff0000"; // or any skin string you like
        w = 40;
        h = 100;
    }

    void flagged()
    { // call when the player reaches it
        if (isUp)
        {
            isUp = false;
            skin = "color:#0000ff"; // swap to “down” sprite
        }
    }

    bool up() const { return isUp; }
    bool down() const { return !isUp; }
};

struct Scene
{
    std::string name{"main"};
    std::vector<Platform> platforms{};
    std::vector<Entity> entities{};

    std::vector<Platform> visiblePlatforms(float camX, float camW) const
    {
        std::vector<Platform> out;
        for (auto &p : platforms)
            if (p.x + p.w > camX && p.x < camX + camW)
                out.push_back(p);
        return out;
    }
    std::vector<Entity *> visibleEntitiesPtr(float camX, float camW)
    {
        std::vector<Entity *> out;
        for (auto &e : entities)
            if (e.type == "player" ||
                (e.x + e.w > camX && e.x < camX + camW))
                out.push_back(&e); // pointer to original
        return out;
    }
};

class Game
{
public:
    std::vector<Scene> scenes{};
    int active_scene{0};
    Game() = default;

    void parseWorld(const std::string &)
    {
        scenes.clear();

        Scene s;

        // ground platform spanning the bottom of the screen
        for (int i = 0; i < 25; i++)
        {
            Platform ground;
            ground.x = i * 50;
            ground.y = 100 + (i * 20);
            ground.bouncy = 0.5f;
            ground.w = 66;
            ground.h = 20;
            ground.type = 0;
            s.platforms.push_back(ground);
        }

        // placeholder player entity
        Entity player;
        player.type = "player";
        player.skin = "color:#ee22ff";
        player.x = 10;
        player.y = 10;
        player.dx = 50;
        player.ddy = 400; // gravity
        s.entities.push_back(player);

        // placeholder player entity
        Flagpole pole; // ctor sets skin + size
        pole.x = 450;  // spawn position
        pole.y = 200;
        s.entities.push_back(pole); // push by value; still an Entity

        scenes.push_back(s);
    }

    uint32_t last_tick = -1;


    void main_tick()
    {
        uint32_t now = millis();
        if (last_tick == -1)
        {
            last_tick = now;
        }
        uint32_t delta = now - last_tick;
        if (delta > 1000)
            delta = 1000;
        last_tick = now;

        Scene &scene = scenes[active_scene];

        auto platforms = scene.visiblePlatforms(cameraX, cameraWidth);
        auto entities = scene.visibleEntitiesPtr(cameraX, cameraWidth);

        const float EPS_Margin = 0.01f; // tiny gap to avoid false hits

        for (auto *e : entities)
        {
            /* -----------------------------------------
               1. integrate forces
            ------------------------------------------*/
            float dt = delta * 1e-3f; // ms → s
            e->dx += e->ddx * dt;
            e->dy += e->ddy * dt;

            float newX = e->x + e->dx * dt; // swept end-points
            float newY = e->y + e->dy * dt;

            /* -----------------------------------------
               2. vertical collisions (Y FIRST)
            ------------------------------------------*/
            for (const Platform &p : platforms)
            {
                if (p.type == 2)
                    continue; // noclip
                if (e->x + e->w <= p.x || e->x >= p.x + p.w)
                    continue; // no X overlap

                /* -------- one-way TOP platform (type 1) -------- */
                if (p.type == 1 && e->dy > 0) // falling
                {
                    bool startAbove = e->y + e->h <= p.y - EPS_Margin;
                    bool crossesTop = newY + e->h >= p.y;
                    if (startAbove && crossesTop)
                    {
                        newY = p.y - e->h;
                        e->dy = -e->dy * p.bouncy;
                    }
                    continue; // nothing else to test
                }

                /* -------- fully SOLID (type 0) -------- */
                // Only test if we actually overlap after the move
                if (newX + e->w <= p.x || newX >= p.x + p.w ||
                    newY + e->h <= p.y || newY >= p.y + p.h)
                    continue;

                // penetration depths
                float penTop = (p.y) - (e->y + e->h);   // >0 if above
                float penBottom = (e->y) - (p.y + p.h); // >0 if below
                float penLeft = (p.x) - (e->x + e->w);
                float penRight = (e->x) - (p.x + p.w);

                // pick shallowest axis
                if (fabsf(penTop) < fabsf(penBottom) &&
                    fabsf(penTop) < fabsf(penLeft) &&
                    fabsf(penTop) < fabsf(penRight))
                { // hit top
                    newY = p.y - e->h;
                    e->dy = -e->dy * p.bouncy;
                }
                else if (fabsf(penBottom) < fabsf(penLeft) &&
                         fabsf(penBottom) < fabsf(penRight))
                { // hit bottom
                    newY = p.y + p.h;
                    e->dy = -e->dy * p.bouncy;
                }
                else if (fabsf(penLeft) < fabsf(penRight))
                { // hit left face
                    newX = p.x - e->w;
                    e->dx = -e->dx * p.bouncy;
                }
                else
                { // hit right face
                    newX = p.x + p.w;
                    e->dx = -e->dx * p.bouncy;
                }
            }

            /* -----------------------------------------
               3. commit position
            ------------------------------------------*/
            e->x = newX;
            e->y = newY;

            /* -----------------------------------------
               4. flagpole trigger
            ------------------------------------------*/
            if (e->type == "player")
            {
                for (auto *other : entities)
                {
                    if (other->type != "flagpole")
                        continue;
                    Flagpole *fp = static_cast<Flagpole *>(other);
                    if (fp->down())
                        break; // already done
                    bool hit = !(e->x + e->w <= fp->x || e->x >= fp->x + fp->w ||
                                 e->y + e->h <= fp->y || e->y >= fp->y + fp->h);
                    if (hit)
                    {
                        fp->flagged();
                        break;
                    }
                }
            }

            /* -----------------------------------------
               5. debug
            ------------------------------------------*/
        }
    }

    const std::vector<Scene> &
    getScenes() const
    {
        return scenes;
    }

    // camera position, used for rendering
    float cameraWidth{480};
    float cameraHeight{320};
    float cameraX{0};
    float cameraY{0};
};