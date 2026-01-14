#pragma once

#include "entity.hpp"
#include "../ui/widget.hpp"
#include "../ui/grid.hpp"
#include "../input/input.hpp"

namespace game {

enum class GameStatus {
    Running,
    Paused,
    LevelCompleted,
    Finished,
    GameOver
};

class Game : public ui::Widget {
public:
    struct Bounds { int w, h; };

    Game(int width, int height, int tps);

    // Widget interface
    void draw(tui::Screen& screen, ui::BBox bbox) override;

    // Game logic
    void update(i64 deltaTime);
    void processInput(const input::Event& ev);
    void removeDeadEntities();
    void reset();  // Reset game for new game

    // Entity spawning
    Player& spawnPlayer(int x, int y, int health, int dmg, int cooldown);
    Enemy& spawnEnemy(int x, int y, int health, int score, int fireFreq, int dmg);
    Bullet& spawnBullet(int x, int y, int dmg, EntityType owner);

    // Accessors
    GameStatus status() const { return m_status; }
    void setStatus(GameStatus s) { m_status = s; }

    const int& score() const { return m_score; }
    void addScore(int s) { m_score += s; }
    const int& level() const { return m_level; }
    void setLevel(int l) { m_level = l; }
    void incrementLevel() { m_level++; }
    int tps() const { return m_tps; }

    Player* player() { return m_player.get(); }
    const Player* player() const { return m_player.get(); }

    std::vector<std::unique_ptr<Enemy>>& enemies() { return m_enemies; }
    const std::vector<std::unique_ptr<Enemy>>& enemies() const { return m_enemies; }

    std::vector<std::unique_ptr<Bullet>>& bullets() { return m_bullets; }

    // For stats panel binding
    int bulletCount() const { return static_cast<int>(m_bullets.size()); }
    int enemyCount() const { return static_cast<int>(m_enemies.size()); }

    Bounds bounds() const { return m_bounds; }

    // Statistics
    int shotsFired() const { return m_shotsFired; }
    int shotsHit() const { return m_shotsHit; }
    int kills() const { return m_kills; }
    int timeSeconds() const { return static_cast<int>(m_elapsedTime / 1000000LL); }
    int accuracyPercent() const { return m_shotsFired > 0 ? (m_shotsHit * 100) / m_shotsFired : 0; }

    void incrementShotsFired() { m_shotsFired++; }
    void incrementShotsHit() { m_shotsHit++; }
    void incrementKills() { m_kills++; }

private:
    void placeEntitiesOnGrid();

    Bounds m_bounds;
    int m_level = 1;
    int m_score = 0;
    GameStatus m_status = GameStatus::Running;

    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<std::unique_ptr<Bullet>> m_bullets;

    std::unique_ptr<ui::Grid> m_grid;
    int m_tps;

    // Statistics
    int m_shotsFired = 0;
    int m_shotsHit = 0;
    int m_kills = 0;
    i64 m_elapsedTime = 0;
};

} // namespace game
