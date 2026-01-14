#pragma once

#include "../common.hpp"
#include "../tui/screen.hpp"

namespace game {

enum class EntityType {
    Player,
    Enemy,
    Bullet
};

// Entity colors that map to screen colors
enum class EntityColor {
    None,
    Red,
    Green,
    Blue,
    Cyan,
    Yellow
};

inline tui::Color toScreenColor(EntityColor c) {
    switch (c) {
        case EntityColor::Red:    return tui::Color::Red();
        case EntityColor::Green:  return tui::Color::Green();
        case EntityColor::Blue:   return tui::Color::Blue();
        case EntityColor::Cyan:   return tui::Color::Cyan();
        case EntityColor::Yellow: return tui::Color::Yellow();
        default:                  return tui::Color::White();
    }
}

// Forward declaration
class Game;

// Base entity with virtual methods
class Entity {
public:
    virtual ~Entity() = default;

    // Virtual methods (replace function pointers from C version)
    virtual int damage(int amount) { return 0; }
    virtual void update() {}

    // Draw entity at screen coordinates
    void draw(tui::Screen& screen, int x, int y) const;

    // Accessors
    int x() const { return m_x; }
    int y() const { return m_y; }
    void setPos(int x, int y) { m_x = x; m_y = y; }
    void move(int dx, int dy) { m_x += dx; m_y += dy; }

    bool isAlive() const { return m_alive; }
    void kill() { m_alive = false; }
    void setAlive(bool alive) { m_alive = alive; }

    EntityType type() const { return m_type; }
    const std::string& shape() const { return m_shape; }
    void setShape(std::string s) { m_shape = std::move(s); }

    EntityColor color() const { return m_color; }
    void setColor(EntityColor c) { m_color = c; }

protected:
    Entity(int x, int y, EntityType type, std::string shape);

    int m_x, m_y;
    bool m_alive = true;
    EntityType m_type;
    std::string m_shape;
    EntityColor m_color = EntityColor::None;
};

// Check collision between two entities
bool collision(const Entity& a, const Entity& b);

// Player entity
class Player : public Entity {
public:
    Player(int x, int y, int health, int dmg, int cooldown);

    int damage(int amount) override;
    void update() override;

    bool canFire() const { return m_ticks >= m_cooldown; }
    void resetFireCooldown() { m_ticks = 0; }
    const int& health() const { return m_health; }
    void setHealth(int h) { m_health = h; m_ticks = m_cooldown; }
    int attackDamage() const { return m_damage; }

private:
    int m_health;
    int m_damage;
    int m_cooldown;
    int m_ticks;
    bool m_damaged = false;
};

// Enemy entity
class Enemy : public Entity {
public:
    Enemy(int x, int y, int health, int score, int fireFreq, int dmg);

    int damage(int amount) override;
    void update() override;

    void setGame(Game* game) { m_game = game; }
    int scoreValue() const { return m_score; }

private:
    int m_health;
    int m_score;
    int m_fireFreq;
    int m_lastFired = 0;
    int m_damage;
    Game* m_game = nullptr;  // non-owning reference
};

// Bullet entity
class Bullet : public Entity {
public:
    Bullet(int x, int y, int dmg, EntityType owner);

    void update() override;

    void setGame(Game* game) { m_game = game; }
    EntityType owner() const { return m_owner; }
    int bulletDamage() const { return m_damage; }

private:
    EntityType m_owner;
    int m_damage;
    int m_moveFreq = 1;
    int m_lastMoved = 0;
    Game* m_game = nullptr;
};

} // namespace game
