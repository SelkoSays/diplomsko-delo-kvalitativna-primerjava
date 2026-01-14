//! Space Invaders - Rust Implementation

mod common;
mod game;
mod input;
mod tui;
mod ui;

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

use crate::common::US_PER_SEC;
use crate::game::{
    game::{Game, GameStatus},
    level::{LEVEL_COUNT, LEVELS, spawn_level},
};
use crate::input::{Event, KeyCode, is_char, is_key, poll};
use crate::tui::screen::Screen;
use crate::ui::{
    frame::{Frame, FrameSplit},
    menu::Menu,
    panel::Panel,
    widget::{BBox, Widget},
};

/// Application screen types
#[derive(Clone, Copy, PartialEq, Eq)]
enum ScreenType {
    Game,
    Menu,
    GameOver,
    Win,
}

/// Game widget wrapper
struct GameWidget {
    game: Arc<Mutex<Game>>,
}

impl Widget for GameWidget {
    fn draw(&self, screen: &mut Screen, bbox: BBox) {
        if let Ok(game) = self.game.lock() {
            game.draw(screen, bbox);
        }
    }
}

/// Application state
struct App {
    screen: Screen,
    running: Arc<AtomicBool>,
    current_screen: ScreenType,

    frame: Frame,
    menu: Menu,
    game_over_menu: Menu,
    win_menu: Menu,

    game: Arc<Mutex<Game>>,

    should_start_new_game: Arc<AtomicBool>,
    should_continue: Arc<AtomicBool>,
    should_quit: Arc<AtomicBool>,
}

impl App {
    fn new() -> common::Result<Self> {
        let screen = Screen::new()?;
        let game = Arc::new(Mutex::new(Game::new(11, 11, 4)));

        {
            let mut g = game.lock().unwrap();
            spawn_level(&mut g, &LEVELS[0]);
        }

        let running = Arc::new(AtomicBool::new(true));
        let should_start_new_game = Arc::new(AtomicBool::new(false));
        let should_continue = Arc::new(AtomicBool::new(false));
        let should_quit = Arc::new(AtomicBool::new(false));

        let mut app = App {
            screen,
            running,
            current_screen: ScreenType::Game,
            frame: Frame::new(0, 0, 0, 0),
            menu: Menu::new(),
            game_over_menu: Menu::new(),
            win_menu: Menu::new(),
            game,
            should_start_new_game,
            should_continue,
            should_quit,
        };

        app.setup_layout();
        app.setup_menus();

        Ok(app)
    }

    fn setup_layout(&mut self) {
        let w = self.screen.width() as u32;
        let h = self.screen.height() as u32;

        self.frame = Frame::new(w, h, 0, 0);

        let (top_frame, bottom_frame) = self
            .frame
            .split(h.saturating_sub(4), FrameSplit::Horizontal);
        let (game_frame, stats_frame) = top_frame.split(w * 2 / 3, FrameSplit::Vertical);

        let mut controls = Panel::new();
        controls.add_empty_line();
        controls.add_text(" Controls:    [<] / [a] Left    [>] / [d] Right    [space] Shoot    [q] Quit    [m] Menu");
        bottom_frame.add_widget(Box::new(controls));

        let mut stats = Panel::new();
        let g = Arc::clone(&self.game);
        stats.add_value(" Level", {
            let g = Arc::clone(&g);
            move || g.lock().map(|g| g.level().to_string()).unwrap_or_default()
        });
        stats.add_value(" Score", {
            let g = Arc::clone(&g);
            move || g.lock().map(|g| g.score().to_string()).unwrap_or_default()
        });
        stats.add_value(" Lives", {
            let g = Arc::clone(&g);
            move || {
                g.lock()
                    .map(|g| g.player_health().to_string())
                    .unwrap_or_default()
            }
        });
        stats.add_empty_line();
        stats.add_value(" Kills", {
            let g = Arc::clone(&g);
            move || g.lock().map(|g| g.kills().to_string()).unwrap_or_default()
        });
        stats.add_value(" Accuracy", {
            let g = Arc::clone(&g);
            move || {
                g.lock()
                    .map(|g| format!("{}%", g.accuracy_percent()))
                    .unwrap_or_default()
            }
        });
        stats.add_value(" Time", {
            let g = Arc::clone(&g);
            move || {
                g.lock()
                    .map(|g| {
                        let secs = g.time_seconds();
                        let mins = secs / 60;
                        let s = secs % 60;
                        if mins > 0 {
                            format!("{}:{:02}", mins, s)
                        } else {
                            format!("{}s", s)
                        }
                    })
                    .unwrap_or_default()
            }
        });
        stats.add_empty_line();
        stats.add_value(" # Bullets on screen", {
            let g = Arc::clone(&g);
            move || {
                g.lock()
                    .map(|g| g.bullet_count().to_string())
                    .unwrap_or_default()
            }
        });
        stats.add_value(" # Enemies remaining", {
            let g = Arc::clone(&g);
            move || {
                g.lock()
                    .map(|g| g.enemy_count().to_string())
                    .unwrap_or_default()
            }
        });
        stats_frame.add_widget(Box::new(stats));

        game_frame.add_widget(Box::new(GameWidget {
            game: Arc::clone(&self.game),
        }));
    }

    fn setup_menus(&mut self) {
        self.menu.add_entry::<fn()>("Menu:", false, None);
        self.menu.add_separator();

        let continue_flag = Arc::clone(&self.should_continue);
        self.menu.add_entry(
            "Continue",
            true,
            Some(move || {
                continue_flag.store(true, Ordering::Relaxed);
            }),
        );

        let new_game_flag = Arc::clone(&self.should_start_new_game);
        self.menu.add_entry(
            "New Game",
            true,
            Some(move || {
                new_game_flag.store(true, Ordering::Relaxed);
            }),
        );

        let quit_flag = Arc::clone(&self.should_quit);
        self.menu.add_entry(
            "Quit",
            true,
            Some(move || {
                quit_flag.store(true, Ordering::Relaxed);
            }),
        );

        self.game_over_menu
            .add_entry::<fn()>("Game Over!", false, None);
        self.game_over_menu.add_separator();

        let new_game_flag = Arc::clone(&self.should_start_new_game);
        self.game_over_menu.add_entry(
            "New Game",
            true,
            Some(move || {
                new_game_flag.store(true, Ordering::Relaxed);
            }),
        );

        let quit_flag = Arc::clone(&self.should_quit);
        self.game_over_menu.add_entry(
            "Quit",
            true,
            Some(move || {
                quit_flag.store(true, Ordering::Relaxed);
            }),
        );

        self.win_menu.add_entry::<fn()>("You Win!", false, None);
        self.win_menu.add_separator();

        let new_game_flag = Arc::clone(&self.should_start_new_game);
        self.win_menu.add_entry(
            "New Game",
            true,
            Some(move || {
                new_game_flag.store(true, Ordering::Relaxed);
            }),
        );

        let quit_flag = Arc::clone(&self.should_quit);
        self.win_menu.add_entry(
            "Quit",
            true,
            Some(move || {
                quit_flag.store(true, Ordering::Relaxed);
            }),
        );
    }

    fn process_input(&mut self, event: &Event) {
        let Event::Key(key) = event else { return };

        if is_char(key, 'q') {
            self.running.store(false, Ordering::Relaxed);
            return;
        }

        match self.current_screen {
            ScreenType::Game => {
                if is_char(key, 'm') {
                    if let Ok(mut g) = self.game.lock() {
                        g.set_status(GameStatus::Paused);
                    }
                    self.current_screen = ScreenType::Menu;
                    return;
                }
                if let Ok(mut g) = self.game.lock() {
                    g.process_input(event);
                }
            }
            ScreenType::Menu => {
                if is_key(key, KeyCode::Up) {
                    self.menu.move_up();
                } else if is_key(key, KeyCode::Down) {
                    self.menu.move_down();
                } else if is_key(key, KeyCode::Enter) {
                    self.menu.select();
                }
            }
            ScreenType::GameOver => {
                if is_key(key, KeyCode::Up) {
                    self.game_over_menu.move_up();
                } else if is_key(key, KeyCode::Down) {
                    self.game_over_menu.move_down();
                } else if is_key(key, KeyCode::Enter) {
                    self.game_over_menu.select();
                }
            }
            ScreenType::Win => {
                if is_key(key, KeyCode::Up) {
                    self.win_menu.move_up();
                } else if is_key(key, KeyCode::Down) {
                    self.win_menu.move_down();
                } else if is_key(key, KeyCode::Enter) {
                    self.win_menu.select();
                }
            }
        }
    }

    fn process_menu_flags(&mut self) {
        if self.should_quit.swap(false, Ordering::Relaxed) {
            self.running.store(false, Ordering::Relaxed);
        }

        if self.should_continue.swap(false, Ordering::Relaxed) {
            if let Ok(mut g) = self.game.lock() {
                if g.status() == GameStatus::Paused {
                    g.set_status(GameStatus::Running);
                }
            }
            self.current_screen = ScreenType::Game;
        }

        if self.should_start_new_game.swap(false, Ordering::Relaxed) {
            if let Ok(mut g) = self.game.lock() {
                g.reset();
                spawn_level(&mut g, &LEVELS[0]);
            }
            self.current_screen = ScreenType::Game;
        }
    }

    fn render(&mut self) {
        self.screen.clear();

        match self.current_screen {
            ScreenType::Game => {
                self.frame.draw(&mut self.screen);
            }
            ScreenType::Menu => {
                self.menu.draw_on(&mut self.screen);
            }
            ScreenType::GameOver => {
                self.game_over_menu.draw_on(&mut self.screen);
            }
            ScreenType::Win => {
                self.win_menu.draw_on(&mut self.screen);
            }
        }

        let _ = self.screen.flush();
    }
}

unsafe impl Sync for App {}

static mut APP: Option<Mutex<App>> = None;

#[allow(static_mut_refs)]
fn main() -> common::Result<()> {
    unsafe { APP = Some(Mutex::new(App::new()?)) };
    run();
    unsafe { APP.take() };
    Ok(())
}

macro_rules! app_get {
    ($app:ident . $field:ident) => {
        $app.as_mut().unwrap().lock().unwrap().$field
    };
    ($app:ident) => {
        $app.as_mut().unwrap().lock().unwrap()
    };
    (unsafe $app:ident . $field:ident) => {
        (unsafe { $app.as_mut().unwrap().lock().unwrap() }).$field
    };
}

#[allow(static_mut_refs)]
fn running() -> Arc<AtomicBool> {
    app_get!(unsafe APP.running).clone()
}

#[allow(static_mut_refs)]
fn run() {
    unsafe {
        let mut app = app_get!(APP);
        app.render();
    }

    let input_thread = thread::spawn(move || {
        while running().load(Ordering::Relaxed) {
            unsafe {
                let mut app = app_get!(APP);
                if let Some(event) = poll() {
                    app.process_input(&event);
                    app.process_menu_flags();
                    app.render();
                }
            }
            thread::sleep(Duration::from_micros(500));
        }
    });

    let tps = app_get!(unsafe APP.game).lock().unwrap().tps();
    let tick_interval = Duration::from_micros((US_PER_SEC / tps) as u64);

    let game = Arc::clone(&app_get!(unsafe APP.game));

    while running().load(Ordering::Relaxed) {
        unsafe {
            let mut app = app_get!(APP);
            if app.current_screen == ScreenType::Game {
                let mut g = game.lock().unwrap();
                if g.status() == GameStatus::Running {
                    g.update(tick_interval.as_micros() as i64);
                    g.remove_dead_entities();

                    if g.enemy_count() == 0 {
                        if let Some(player) = g.player() {
                            if player.is_alive() {
                                g.increment_level();
                                let level = g.level();
                                if level > LEVEL_COUNT {
                                    g.set_status(GameStatus::Finished);
                                    app.current_screen = ScreenType::Win;
                                } else {
                                    spawn_level(&mut g, &LEVELS[(level - 1) as usize]);
                                }
                            }
                        }
                    }

                    if g.status() == GameStatus::GameOver {
                        app.current_screen = ScreenType::GameOver;
                    }
                }
            }

            app.render();
        }

        thread::sleep(tick_interval);
    }

    running().store(false, Ordering::Relaxed);
    let _ = input_thread.join();
}
