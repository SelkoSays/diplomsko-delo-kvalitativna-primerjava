#include "entity.hpp"
#include "game.hpp"

namespace game {

Entity::Entity(int x, int y, EntityType type, std::string shape)
    : m_x(x), m_y(y), m_type(type), m_shape(std::move(shape)) {}

void Entity::draw(tui::Screen& screen, int x, int y) const {
    if (!m_alive) return;

    auto fg = toScreenColor(m_color);
    screen.putChar(x, y, m_shape);
    screen.setFgColor(x, y, fg);
}

bool collision(const Entity& a, const Entity& b) {
    return a.x() == b.x() && a.y() == b.y();
}

Player::Player(int x, int y, int health, int dmg, int cooldown)
    : Entity(x, y, EntityType::Player, "A")
    , m_health(health)
    , m_damage(dmg)
    , m_cooldown(cooldown)
    , m_ticks(cooldown) {
    m_color = EntityColor::Cyan;
}

int Player::damage(int amount) {
    if (m_health <= amount) {
        m_health = 0;
        kill();
    } else {
        m_health -= amount;
    }
    m_damaged = true;
    m_color = EntityColor::None;
    return 0;
}

void Player::update() {
    m_ticks++;
    if (m_damaged) {
        m_damaged = false;
    } else {
        m_color = EntityColor::Cyan;
    }
}

Enemy::Enemy(int x, int y, int health, int score, int fireFreq, int dmg)
    : Entity(x, y, EntityType::Enemy, "V")
    , m_health(health)
    , m_score(score)
    , m_fireFreq(fireFreq)
    , m_damage(dmg) {
    m_color = EntityColor::None;
}

int Enemy::damage(int amount) {
    if (m_health <= amount) {
        m_health = 0;
        kill();
        return m_score;
    }
    m_health -= amount;
    return 0;
}

void Enemy::update() {
    m_color = EntityColor::None;
    m_lastFired++;

    if (m_lastFired >= (m_fireFreq - 1)) {
        m_color = EntityColor::Red;
    }

    if (m_lastFired >= m_fireFreq) {
        if (m_game) {
            auto& bullet = m_game->spawnBullet(m_x, m_y + 1, m_damage, EntityType::Enemy);
            bullet.setShape("|");
        }
        m_lastFired = 0;
    }
}

Bullet::Bullet(int x, int y, int dmg, EntityType owner)
    : Entity(x, y, EntityType::Bullet, "0")
    , m_owner(owner)
    , m_damage(dmg) {
    m_color = EntityColor::None;
}

void Bullet::update() {
    m_lastMoved++;

    if (m_lastMoved < m_moveFreq) {
        return;
    }
    m_lastMoved = 0;

    if (!m_game) return;

    if (m_owner == EntityType::Enemy) {
        Player* player = m_game->player();
        if (player && collision(*this, *player)) {
            player->damage(m_damage);
            kill();
            return;
        }

        // Move down
        if (m_y + 1 >= m_game->bounds().h) {
            kill();
            return;
        }
        m_y++;
    } else {
        for (auto& enemy : m_game->enemies()) {
            if (enemy && collision(*this, *enemy)) {
                int score = enemy->damage(m_damage);
                m_game->addScore(score);
                m_game->incrementShotsHit();
                if (score > 0) {
                    m_game->incrementKills();
                }
                kill();
                return;
            }
        }

        // Move up
        if (m_y - 1 < 0) {
            kill();
            return;
        }
        m_y--;
    }
}

} // namespace game
