#include "common.hpp"
#include "tui/screen.hpp"
#include "input/input.hpp"
#include "ui/frame.hpp"
#include "ui/panel.hpp"
#include "ui/menu.hpp"
#include "game/game.hpp"
#include "game/level.hpp"

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

enum class ScreenType {
    Game,
    Menu,
    GameOver,
    Win
};

class Application {
public:
    Application()
        : m_game(11, 11, 4)
        , m_rootFrame(m_screen.width(), m_screen.height(), 0, 0) {

        setupGame();
        setupLayout();
        setupMenu();
    }

    void run() {
        render();

        std::thread inputThread(&Application::inputLoop, this);

        auto sleepTime = std::chrono::microseconds(US_PER_SEC / m_game.tps());

        while (m_running) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                if (m_game.status() == game::GameStatus::Running) {
                    m_game.update(sleepTime.count());
                    m_game.removeDeadEntities();

                    // Check level completion
                    if (m_game.enemies().empty() && m_game.player() && m_game.player()->isAlive()) {
                        m_game.incrementLevel();
                        if (m_game.level() > game::LEVEL_COUNT) {
                            m_game.setStatus(game::GameStatus::Finished);
                            m_currentScreen = ScreenType::Win;
                        } else {
                            game::spawnLevel(m_game, game::LEVELS[m_game.level() - 1]);
                        }
                    }

                    if (m_game.status() == game::GameStatus::GameOver) {
                        m_currentScreen = ScreenType::GameOver;
                    }
                }

                render();
            }

            std::this_thread::sleep_for(sleepTime);
        }

        m_running = false;
        inputThread.join();
    }

private:
    void setupGame() {
        m_game.spawnPlayer((m_game.bounds().w - 1) / 2,
                           m_game.bounds().h - 1, 5, 1, 2);
        game::spawnLevel(m_game, game::LEVELS[0]);
    }

    void setupLayout() {
        auto split1 = m_rootFrame.split(
            m_rootFrame.height() - 4, ui::FrameSplit::Horizontal);
        ui::Frame& topFrame = split1.first;
        ui::Frame& bottomFrame = split1.second;

        auto split2 = topFrame.split(
            topFrame.width() * 2 / 3, ui::FrameSplit::Vertical);
        ui::Frame& gameFrame = split2.first;
        ui::Frame& statsFrame = split2.second;

        auto controls = std::make_unique<ui::Panel>();
        controls->addEmptyLine();
        controls->addText(" Controls:    [<] / [a] Left    [>] / [d] Right "
                         "   [space] Shoot    [q] Quit    [m] Menu");
        bottomFrame.addWidget(std::move(controls));

        auto stats = std::make_unique<ui::Panel>();
        game::Game* g = &m_game;
        stats->addValue(" Level", [g]() { return std::to_string(g->level()); });
        stats->addValue(" Score", [g]() { return std::to_string(g->score()); });
        if (g->player()) {
            game::Player* p = g->player();
            stats->addValue(" Lives", [p]() { return std::to_string(p->health()); });
        }
        stats->addEmptyLine();
        stats->addValue(" Kills", [g]() { return std::to_string(g->kills()); });
        stats->addValue(" Accuracy", [g]() { return std::to_string(g->accuracyPercent()) + "%"; });
        stats->addValue(" Time", [g]() {
            int secs = g->timeSeconds();
            int mins = secs / 60;
            secs = secs % 60;
            if (mins > 0) {
                return std::to_string(mins) + ":" + (secs < 10 ? "0" : "") + std::to_string(secs);
            }
            return std::to_string(secs) + "s";
        });
        stats->addEmptyLine();
        stats->addValue(" # Bullets on screen", [g]() { return std::to_string(g->bulletCount()); });
        stats->addValue(" # Enemies remaining", [g]() { return std::to_string(g->enemyCount()); });
        statsFrame.addWidget(std::move(stats));

        class GameWidget : public ui::Widget {
        public:
            explicit GameWidget(game::Game* game) : m_game(game) {}
            void draw(tui::Screen& screen, ui::BBox bbox) override {
                if (m_game) m_game->draw(screen, bbox);
            }
        private:
            game::Game* m_game;
        };

        gameFrame.addWidget(std::make_unique<GameWidget>(&m_game));
    }

    void setupMenu() {
        m_menu.addEntry("Menu:", false, nullptr);
        m_menu.addSeparator();
        m_menu.addEntry("Continue", true, [this]() {
            if (m_game.status() == game::GameStatus::Paused) {
                m_game.setStatus(game::GameStatus::Running);
            }
            m_currentScreen = ScreenType::Game;
        });
        m_menu.addEntry("New Game", true, [this]() {
            startNewGame();
        });
        m_menu.addEntry("Quit", true, [this]() {
            m_running = false;
        });

        m_menu.moveDown();

        m_gameOverMenu.addEntry("Game Over!", false, nullptr);
        m_gameOverMenu.addSeparator();
        m_gameOverMenu.addEntry("New Game", true, [this]() {
            startNewGame();
        });
        m_gameOverMenu.addEntry("Quit", true, [this]() {
            m_running = false;
        });

        m_gameOverMenu.moveDown();

        m_winMenu.addEntry("You Win!", false, nullptr);
        m_winMenu.addSeparator();
        m_winMenu.addEntry("New Game", true, [this]() {
            startNewGame();
        });
        m_winMenu.addEntry("Quit", true, [this]() {
            m_running = false;
        });

        m_winMenu.moveDown();
    }

    void startNewGame() {
        m_game.reset();
        game::spawnLevel(m_game, game::LEVELS[0]);
        m_currentScreen = ScreenType::Game;
    }

    void processInput(const input::Event& ev) {
        auto* keyEv = std::get_if<input::KeyEvent>(&ev);
        if (!keyEv) return;

        if (keyEv->isChar('q') || keyEv->isChar('Q')) {
            m_running = false;
            return;
        }

        switch (m_currentScreen) {
        case ScreenType::Game:
            if (keyEv->isChar('m') || keyEv->isChar('M')) {
                m_game.setStatus(game::GameStatus::Paused);
                m_currentScreen = ScreenType::Menu;
                return;
            }
            m_game.processInput(ev);
            break;

        case ScreenType::Menu:
            if (keyEv->isKey(input::KeyCode::Up)) {
                m_menu.moveUp();
            } else if (keyEv->isKey(input::KeyCode::Down)) {
                m_menu.moveDown();
            } else if (keyEv->isKey(input::KeyCode::Enter)) {
                m_menu.select();
            }
            break;

        case ScreenType::GameOver:
            if (keyEv->isKey(input::KeyCode::Up)) {
                m_gameOverMenu.moveUp();
            } else if (keyEv->isKey(input::KeyCode::Down)) {
                m_gameOverMenu.moveDown();
            } else if (keyEv->isKey(input::KeyCode::Enter)) {
                m_gameOverMenu.select();
            }
            break;

        case ScreenType::Win:
            if (keyEv->isKey(input::KeyCode::Up)) {
                m_winMenu.moveUp();
            } else if (keyEv->isKey(input::KeyCode::Down)) {
                m_winMenu.moveDown();
            } else if (keyEv->isKey(input::KeyCode::Enter)) {
                m_winMenu.select();
            }
            break;
        }
    }

    void inputLoop() {
        while (m_running) {
            std::optional<input::Event> ev;

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                ev = m_input.poll();
            }

            if (ev) {
                std::lock_guard<std::mutex> lock(m_mutex);
                processInput(*ev);
                render();
            }

            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }

    void render() {
        m_screen.clear();

        switch (m_currentScreen) {
        case ScreenType::Game:
            m_rootFrame.draw(m_screen);
            break;
        case ScreenType::Menu:
            m_menu.draw(m_screen);
            break;
        case ScreenType::GameOver:
            m_gameOverMenu.draw(m_screen);
            break;
        case ScreenType::Win:
            m_winMenu.draw(m_screen);
            break;
        }

        m_screen.flush();
    }

    std::mutex m_mutex;
    std::atomic<bool> m_running{true};

    tui::Screen m_screen;
    input::InputHandler m_input;

    game::Game m_game;
    ui::Frame m_rootFrame;
    ui::Menu m_menu;
    ui::Menu m_gameOverMenu;
    ui::Menu m_winMenu;

    ScreenType m_currentScreen = ScreenType::Game;
};

int main() {
    Application app;
    app.run();
    return 0;
}
