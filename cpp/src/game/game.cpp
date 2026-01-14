#include "game.hpp"
#include <algorithm>

namespace game {

Game::Game(int width, int height, int tps)
    : m_bounds{width, height}
    , m_grid(std::make_unique<ui::Grid>(width, height))
    , m_tps(tps) {}

Player& Game::spawnPlayer(int x, int y, int health, int dmg, int cooldown) {
    m_player = std::make_unique<Player>(x, y, health, dmg, cooldown);
    return *m_player;
}

Enemy& Game::spawnEnemy(int x, int y, int health, int score, int fireFreq, int dmg) {
    auto enemy = std::make_unique<Enemy>(x, y, health, score, fireFreq, dmg);
    enemy->setGame(this);
    m_enemies.push_back(std::move(enemy));
    return *m_enemies.back();
}

Bullet& Game::spawnBullet(int x, int y, int dmg, EntityType owner) {
    auto bullet = std::make_unique<Bullet>(x, y, dmg, owner);
    bullet->setGame(this);
    m_bullets.push_back(std::move(bullet));
    return *m_bullets.back();
}

void Game::update(i64 deltaTime) {
    m_elapsedTime += deltaTime;

    for (auto& bullet : m_bullets) {
        if (bullet) bullet->update();
    }

    for (auto& enemy : m_enemies) {
        if (enemy) enemy->update();
    }

    if (m_player) {
        m_player->update();
    }
}

void Game::removeDeadEntities() {
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const auto& b) { return !b || !b->isAlive(); }),
        m_bullets.end()
    );

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const auto& e) { return !e || !e->isAlive(); }),
        m_enemies.end()
    );

    if (m_player && !m_player->isAlive()) {
        m_status = GameStatus::GameOver;
    }
}

void Game::reset() {
    m_bullets.clear();
    m_enemies.clear();

    if (m_player) {
        m_player->setHealth(5);
        m_player->setAlive(true);
        m_player->setPos((m_bounds.w - 1) / 2, m_bounds.h - 1);
        m_player->setColor(EntityColor::Cyan);
    }

    m_level = 1;
    m_score = 0;
    m_status = GameStatus::Running;

    m_shotsFired = 0;
    m_shotsHit = 0;
    m_kills = 0;
    m_elapsedTime = 0;
}

void Game::processInput(const input::Event& ev) {
    auto* keyEv = std::get_if<input::KeyEvent>(&ev);
    if (!keyEv) return;

    if (keyEv->isChar('a') || keyEv->isChar('A') ||
        keyEv->isKey(input::KeyCode::Left)) {
        if (m_player && m_player->x() > 0) {
            m_player->move(-1, 0);
        }
    }
    else if (keyEv->isChar('d') || keyEv->isChar('D') ||
             keyEv->isKey(input::KeyCode::Right)) {
        if (m_player && m_player->x() < m_bounds.w - 1) {
            m_player->move(1, 0);
        }
    }
    else if (keyEv->isChar(' ')) {
        if (m_player && m_player->canFire()) {
            spawnBullet(m_player->x(), m_player->y() - 1,
                       m_player->attackDamage(), EntityType::Player);
            m_player->resetFireCooldown();
            m_shotsFired++;
        }
    }
}

void Game::placeEntitiesOnGrid() {
    m_grid->clearCells();

    auto makeDrawFn = [](const Entity* e) {
        return [e](tui::Screen& screen, int x, int y) {
            e->draw(screen, x, y);
        };
    };

    for (const auto& bullet : m_bullets) {
        if (bullet && bullet->isAlive()) {
            auto* cell = m_grid->at(bullet->x(), bullet->y());
            if (cell) {
                cell->setDrawCallback(makeDrawFn(bullet.get()));
            }
        }
    }

    for (const auto& enemy : m_enemies) {
        if (enemy && enemy->isAlive()) {
            auto* cell = m_grid->at(enemy->x(), enemy->y());
            if (cell) {
                cell->setDrawCallback(makeDrawFn(enemy.get()));
            }
        }
    }

    if (m_player && m_player->isAlive()) {
        auto* cell = m_grid->at(m_player->x(), m_player->y());
        if (cell) {
            cell->setDrawCallback(makeDrawFn(m_player.get()));
        }
    }
}

void Game::draw(tui::Screen& screen, ui::BBox bbox) {
    placeEntitiesOnGrid();
    m_grid->draw(screen, bbox);
}

} // namespace game
