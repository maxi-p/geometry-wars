#pragma once

#include "Entity.h"
#include "EntityManager.h"

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig  { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game
{
    sf::RenderWindow            m_window;           // the window we will draw to
    EntityManager               m_entityManager;         // vector of entities to maintain
    sf::Font                    m_font;             // the font we will use to draw
    sf::Text                    m_text;             // the score text to be drawn to the screen
    PlayerConfig                m_playerConfig;
    EnemyConfig                 m_enemyConfig;
    BulletConfig                m_bulletConfig;
    sf::Clock                   m_deltaClock;
    int                         m_score                 = 0;
    int                         m_currentFrame          = 0;
    int                         m_lastEnemySpawnTime    = 0;
    bool                        m_paused                = false; // whether we update game logic
    bool                        m_running               = true;  // whether the game is runnig
    std::shared_ptr<Entity>     m_player;
    bool m_movement = true;
    bool m_lifespan = true;
    bool m_collisions = true;
    bool m_spawning = true;
    bool m_GUI = true;
    bool m_rendering = true;


public:
    Game(const std::string& config);
    void init(const std::string& path);
    void run();
    void setPaused(bool paused);
    void spawnPlayer();
    void spawnEnemy();
    void spawnSmallEnemies(std::shared_ptr<Entity> e);
    void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target);
    void spawnSpecialWeapon(std::shared_ptr<Entity> entity);
    void sMovement();
    void sLifespan();
    void sCollision();
    void sEnemySpawner();
    void sGUI();
    void sRender();
    void sUserInput();

};
