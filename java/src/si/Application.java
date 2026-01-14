package si;

import si.common.Common;
import si.game.Game;
import si.game.GameStatus;
import si.game.Levels;
import si.game.Player;
import si.input.Event;
import si.input.InputHandler;
import si.input.KeyCode;
import si.input.KeyEvent;
import si.tui.Screen;
import si.ui.BBox;
import si.ui.Frame;
import si.ui.FrameSplit;
import si.ui.Menu;
import si.ui.Panel;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantLock;

public final class Application {
    private enum ScreenType {
        GAME,
        MENU,
        GAME_OVER,
        WIN
    }

    private final ReentrantLock mutex = new ReentrantLock();
    private final AtomicBoolean running = new AtomicBoolean(true);

    private Screen screen;
    private InputHandler input;

    private Frame rootFrame;
    private Menu menu;
    private Menu gameOverMenu;
    private Menu winMenu;
    private Game game;

    private ScreenType currentScreen = ScreenType.GAME;

    public Application() {
        screen = new Screen();
        input = new InputHandler();

        setupGame();
        setupLayout();
        setupMenu();
    }

    public void run() {
        render();

        Thread inputThread = new Thread(this::inputLoop, "input-thread");
        inputThread.start();

        long sleepTime = Common.US_PER_SEC / game.tps();

        try {
            while (running.get()) {
                mutex.lock();
                try {
                    if (game.status() == GameStatus.RUNNING) {
                        game.update(sleepTime);
                        game.removeDeadEntities();

                        if (game.enemies().isEmpty() && game.player() != null && game.player().isAlive()) {
                            game.incrementLevel();
                            if (game.level() > Levels.LEVEL_COUNT) {
                                game.setStatus(GameStatus.FINISHED);
                                currentScreen = ScreenType.WIN;
                            } else {
                                Levels.spawnLevel(game, Levels.LEVELS[game.level() - 1]);
                            }
                        }

                        if (game.status() == GameStatus.GAME_OVER) {
                            currentScreen = ScreenType.GAME_OVER;
                        }
                    }

                    render();
                } finally {
                    mutex.unlock();
                }

                Thread.sleep(sleepTime / 1000);
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        running.set(false);
        try {
            inputThread.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        screen.close();
    }

    private void setupGame() {
        game = new Game(11, 11, 4);
        game.spawnPlayer((game.bounds().w() - 1) / 2, game.bounds().h() - 1, 5, 1, 2);
        Levels.spawnLevel(game, Levels.LEVELS[0]);
    }

    private void setupLayout() {
        rootFrame = new Frame(screen.width(), screen.height(), 0, 0);

        Frame[] split1 = rootFrame.split(rootFrame.height() - 4, FrameSplit.HORIZONTAL);
        Frame topFrame = split1[0];
        Frame bottomFrame = split1[1];

        Frame[] split2 = topFrame.split(topFrame.width() * 2 / 3, FrameSplit.VERTICAL);
        Frame gameFrame = split2[0];
        Frame statsFrame = split2[1];

        Panel controls = new Panel();
        controls.addEmptyLine();
        controls.addText(" Controls:    [<] / [a] Left    [>] / [d] Right    [space] Shoot    [q] Quit    [m] Menu");
        bottomFrame.addWidget(controls);

        Panel stats = new Panel();
        stats.addValue(" Level", () -> String.valueOf(game.level()));
        stats.addValue(" Score", () -> String.valueOf(game.score()));
        Player p = game.player();
        if (p != null) {
            stats.addValue(" Lives", () -> String.valueOf(p.health()));
        }
        stats.addEmptyLine();
        stats.addValue(" Kills", () -> String.valueOf(game.kills()));
        stats.addValue(" Accuracy", () -> game.accuracyPercent() + "%");
        stats.addValue(" Time", () -> {
            int secs = game.timeSeconds();
            int mins = secs / 60;
            secs = secs % 60;
            if (mins > 0) {
                return mins + ":" + (secs < 10 ? "0" : "") + secs;
            }
            return secs + "s";
        });
        stats.addEmptyLine();
        stats.addValue(" # Bullets on screen", () -> String.valueOf(game.bulletCount()));
        stats.addValue(" # Enemies remaining", () -> String.valueOf(game.enemyCount()));
        statsFrame.addWidget(stats);

        gameFrame.addWidget((screen, bbox) -> game.draw(screen, bbox));
    }

    private void setupMenu() {
        menu = new Menu();
        menu.addEntry("Menu:", false, null);
        menu.addSeparator();
        menu.addEntry("Continue", true, () -> {
            if (game.status() == GameStatus.PAUSED) {
                game.setStatus(GameStatus.RUNNING);
            }
            currentScreen = ScreenType.GAME;
        });
        menu.addEntry("New Game", true, this::startNewGame);
        menu.addEntry("Quit", true, () -> running.set(false));
        menu.moveDown();

        gameOverMenu = new Menu();
        gameOverMenu.addEntry("Game Over!", false, null);
        gameOverMenu.addSeparator();
        gameOverMenu.addEntry("New Game", true, this::startNewGame);
        gameOverMenu.addEntry("Quit", true, () -> running.set(false));
        gameOverMenu.moveDown();

        winMenu = new Menu();
        winMenu.addEntry("You Win!", false, null);
        winMenu.addSeparator();
        winMenu.addEntry("New Game", true, this::startNewGame);
        winMenu.addEntry("Quit", true, () -> running.set(false));
        winMenu.moveDown();
    }

    private void startNewGame() {
        game.reset();
        Levels.spawnLevel(game, Levels.LEVELS[0]);
        currentScreen = ScreenType.GAME;
    }

    private void processInput(Event ev) {
        if (!(ev instanceof KeyEvent keyEv)) {
            return;
        }

        if (keyEv.isChar('q') || keyEv.isChar('Q')) {
            running.set(false);
            return;
        }

        switch (currentScreen) {
            case GAME -> {
                if (keyEv.isChar('m') || keyEv.isChar('M')) {
                    game.setStatus(GameStatus.PAUSED);
                    currentScreen = ScreenType.MENU;
                    return;
                }
                game.processInput(ev);
            }
            case MENU -> {
                if (keyEv.isKey(KeyCode.UP)) {
                    menu.moveUp();
                } else if (keyEv.isKey(KeyCode.DOWN)) {
                    menu.moveDown();
                } else if (keyEv.isKey(KeyCode.ENTER)) {
                    menu.select();
                }
            }
            case GAME_OVER -> {
                if (keyEv.isKey(KeyCode.UP)) {
                    gameOverMenu.moveUp();
                } else if (keyEv.isKey(KeyCode.DOWN)) {
                    gameOverMenu.moveDown();
                } else if (keyEv.isKey(KeyCode.ENTER)) {
                    gameOverMenu.select();
                }
            }
            case WIN -> {
                if (keyEv.isKey(KeyCode.UP)) {
                    winMenu.moveUp();
                } else if (keyEv.isKey(KeyCode.DOWN)) {
                    winMenu.moveDown();
                } else if (keyEv.isKey(KeyCode.ENTER)) {
                    winMenu.select();
                }
            }
        }
    }

    private void inputLoop() {
        while (running.get()) {
            Event ev = null;

            mutex.lock();
            try {
                ev = input.poll().orElse(null);
            } finally {
                mutex.unlock();
            }

            if (ev != null) {
                mutex.lock();
                try {
                    processInput(ev);
                    render();
                } finally {
                    mutex.unlock();
                }
            }

            try {
                Thread.sleep(0, 500_000);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                break;
            }
        }
    }

    private void render() {
        screen.clear();

        switch (currentScreen) {
            case GAME -> rootFrame.draw(screen);
            case MENU -> menu.draw(screen);
            case GAME_OVER -> gameOverMenu.draw(screen);
            case WIN -> winMenu.draw(screen);
        }

        screen.flush();
    }
}
