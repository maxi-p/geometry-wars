#include "Game.h"
#include "math.h"
#include "string.h"
#include <iostream>
#include <fstream>

Game::Game(const std::string& config)
{
    init(config);
}

void Game::init(const std::string& path)
{
    std::ifstream configFile;
    configFile.open(path);
    std::string category;
    while(configFile >> category)
    {
        if(strcmp(category.c_str(),"Window") == 0)
        {
            float wWidth, wHeight, fps, mode;
            configFile >> wWidth >> wHeight >> fps >> mode;
            m_window.create(sf::VideoMode(wWidth, wHeight), "Geometry Wars");
            m_window.setFramerateLimit(fps);
        }
        if(strcmp(category.c_str(),"Font") == 0)
        {
            std::string fontPath;
            int fontSize, fontR, fontG, fontB;
            configFile >> fontPath >> fontSize >> fontR >> fontG >> fontB;
            if(!m_font.loadFromFile(fontPath)) 
            {
                std::cerr << fontPath;
                std::cerr << "could not load font!\n";
                exit(-1);
            }
        }
        if(strcmp(category.c_str(),"Player") == 0)
        {
            configFile >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
        }
        if(strcmp(category.c_str(),"Enemy") == 0)
        {
            configFile >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
        }
        if(strcmp(category.c_str(),"Bullet") == 0)
        {
            configFile >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
        }
    }

    ImGui::SFML::Init(m_window);

    spawnPlayer();
}

void Game::run()
{
    // TODO: add pause functionality in here
    // some systems should function while paused (rendering)
    // some systems shouldn't (movement/ input)
    while (m_running)
    {
        // update the entity manager
        m_entityManager.update();

        // required update call to imgui
        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        sEnemySpawner();
        sMovement();
        sCollision();
        sUserInput();
        sLifespan();
        sGUI();
        sRender();

        // increment the current frame
        // may need to be moved when pause implemented
        m_currentFrame++;
    }
}

void Game::setPaused(bool paused)
{
    // TODO:
}

void Game::spawnPlayer()
{
    auto entity = m_entityManager.addEntity("player");
    entity->cTransform = std::make_shared<CTransform>(Vec2(m_window.getSize().x/2, m_window.getSize().y/2), Vec2(m_playerConfig.S, m_playerConfig.S), 0.0f);
    entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
    entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
    entity->cInput = std::make_shared<CInput>();
    m_player = entity;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
    // TODO: make sure the enemy is spawned properly with the m_enemyyConfig variables
    // - the enemy must be spawned comletely within the bounds of the window
    auto enemy = m_entityManager.addEntity("enemy");
    enemy->cTransform = std::make_shared<CTransform>(Vec2(200.0f, 200.0f), Vec2(m_enemyConfig.SMAX, m_enemyConfig.SMAX), 0.0f);
    enemy->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
    enemy->cShape = std::make_shared<CShape>(m_enemyConfig.SR, m_enemyConfig.VMAX, sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
    enemy->cInput = std::make_shared<CInput>();

    // record when the most recent enemy was spawned
    m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
    auto bullet = m_entityManager.addEntity("bullet");
    Vec2 shooter(entity->cTransform->pos.x,entity->cTransform->pos.y);
    Vec2 path = target - shooter;
    float length = sqrt(path.x*path.x + path.y*path.y);
    path /= length;
    path *= m_bulletConfig.S;
    bullet->cTransform = std::make_shared<CTransform>(shooter, path, 0.0f);
    bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
    bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
    bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{

}

void Game::sMovement()
{
    for( auto entity : m_entityManager.getEntities())
    {
        if(entity->id() != m_player->id())
        {
            entity->cTransform->pos.x += entity->cTransform->velocity.x;
            entity->cTransform->pos.y += entity->cTransform->velocity.y;
        }
    }
    
    // Player Movement
    {
        if(m_player->cInput->up && m_player->cInput->left)
        {
            m_player->cTransform->pos.y -= m_player->cTransform->velocity.y*0.707f;
            m_player->cTransform->pos.x -= m_player->cTransform->velocity.x*0.707f;

        }
        else if(m_player->cInput->up && m_player->cInput->right)
        {
            m_player->cTransform->pos.y -= m_player->cTransform->velocity.y*0.707f;
            m_player->cTransform->pos.x += m_player->cTransform->velocity.x*0.707f;
        }
        else if(m_player->cInput->down && m_player->cInput->left)
        {
            m_player->cTransform->pos.y += m_player->cTransform->velocity.y*0.707f;
            m_player->cTransform->pos.x -= m_player->cTransform->velocity.x*0.707f;
        }
        else if(m_player->cInput->down && m_player->cInput->right)
        {
            m_player->cTransform->pos.y += m_player->cTransform->velocity.y*0.707f;
            m_player->cTransform->pos.x += m_player->cTransform->velocity.x*0.707f;
        }
        else if(m_player->cInput->up)
        {
            m_player->cTransform->pos.y -= m_player->cTransform->velocity.y;
        }
        else if(m_player->cInput->down)
        {
            m_player->cTransform->pos.y += m_player->cTransform->velocity.y;
        }
        else if(m_player->cInput->left)
        {
            m_player->cTransform->pos.x -= m_player->cTransform->velocity.x;
        }
        else if(m_player->cInput->right)
        {
            m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
        }
    }
}

void Game::sLifespan()
{
    for( auto entity : m_entityManager.getEntities())
    {
        if( entity->cLifespan != NULL)
        {
            entity->cLifespan->remaining--;
            std::cout << "lifespan left: " << entity->cLifespan->remaining << "\n";
            if(entity->cLifespan->remaining <= 0)
            {
                entity->destroy();
            }
        }
    }
}

void Game::sCollision()
{
    for (auto entity : m_entityManager.getEntities("enemy"))
    {
        // Window bouncing system
        Vec2 bottom = entity->cTransform->pos + entity->cCollision->radius;
        Vec2 top    = entity->cTransform->pos - entity->cCollision->radius;
        if(bottom.x >= m_window.getSize().x || top.x <= 0)
        {
            entity->cTransform->velocity *= Vec2(-1.0f, 1.0);
        }        
        if(bottom.y >= m_window.getSize().y || top.y <= 0)
        {
            entity->cTransform->velocity *= Vec2(1.0f, -1.0f);
        }

        // Player to Enemy Collision
        if(entity->cTransform->pos.dist(m_player->cTransform->pos) <= (m_playerConfig.CR + m_enemyConfig.CR)*(m_playerConfig.CR + m_enemyConfig.CR))
        {
            m_player->destroy();
            spawnPlayer();
        }

        // Enemy to Bullet Collision
        for(auto bullet : m_entityManager.getEntities("bullet") )
        {
            if(entity->cTransform->pos.dist(bullet->cTransform->pos) <= (m_enemyConfig.CR + m_bulletConfig.CR)*(m_enemyConfig.CR + m_bulletConfig.CR))
            {
                entity->destroy();
                bullet->destroy();
            }
        }
    }
            
}

void Game::sEnemySpawner()
{
    if(m_currentFrame - m_lastEnemySpawnTime > m_enemyConfig.SI)
        {
            std::cout << "Enemy spawned\n";
            spawnEnemy();
        }
}

void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");

    ImGui::Text("Stuff Goes Here");

    ImGui::End();
}

void Game::sRender()
{
    m_window.clear();

    for( auto entity : m_entityManager.getEntities())
    {
        entity->cShape->circle.setPosition(entity->cTransform->pos.x, entity->cTransform->pos.y);
        entity->cTransform->angle += 1.0f;
        entity->cShape->circle.setRotation(entity->cTransform->angle);
        m_window.draw(entity->cShape->circle);
    }

    ImGui::SFML::Render(m_window);

    m_window.display();
}

void Game::sUserInput()
{
    sf::Event event;

    while(m_window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(m_window, event);

        if(event.type == sf::Event::Closed)
        {
            m_running = false;
        }

        if(event.type == sf::Event::KeyPressed)
        {
            if(event.key.code == sf::Keyboard::W)
            {
                m_player->cInput->up = true;
            }
            if(event.key.code == sf::Keyboard::S)
            {
                m_player->cInput->down = true;
            }
            if(event.key.code == sf::Keyboard::A)
            {
                m_player->cInput->left = true;
            }
            if(event.key.code == sf::Keyboard::D)
            {
                m_player->cInput->right = true;
            }
            if(event.key.code == sf::Keyboard::Q)
            {
                m_running = false;
            }
        }

        if(event.type == sf::Event::KeyReleased)
        {
            if(event.key.code == sf::Keyboard::W)
            {
                m_player->cInput->up = false;
            }
            if(event.key.code == sf::Keyboard::S)
            {
                m_player->cInput->down = false;
            }
            if(event.key.code == sf::Keyboard::A)
            {
                m_player->cInput->left = false;
            }
            if(event.key.code == sf::Keyboard::D)
            {
                m_player->cInput->right = false;
            }
        }

        if(event.type == sf::Event::MouseButtonPressed)
        {
            if(ImGui::GetIO().WantCaptureMouse) {continue;}

            if(event.mouseButton.button == sf::Mouse::Left)
            {

                spawnBullet(m_player, Vec2(event.mouseButton.x,event.mouseButton.y));
                std::cout << "Left Mouse Button CLicked at " << event.mouseButton.x << ", " << event.mouseButton.y << "\n";
            }

            if(event.mouseButton.button == sf::Mouse::Right)
            {
                std::cout << "Right Mouse Clicked\n";
            }
        }

    }
}
