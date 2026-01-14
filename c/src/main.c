#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "game/game.h"
#include "game/level.h"
#include "input/input.h"
#include "tui/screen.h"
#include "ui/frame.h"
#include "ui/menu.h"
#include "ui/panel.h"

typedef enum ScreenType {
    SCREEN_GAME,
    SCREEN_MENU,
    SCREEN_GAME_OVER,
    SCREEN_WIN,
} ScreenType;

typedef struct App {
    // Screen and rendering
    Screen *screen;
    pthread_mutex_t mutex;

    // Application state
    bool running;
    ScreenType current_screen;

    // Input
    InputBuffer input;

    // UI
    Frame *frame;
    Menu menu;
    Menu game_over_menu;
    Menu win_menu;

    // Game
    Game game;
} App;

// Forward declarations
static void app_render(App *app);

void app_cleanup(App *app) {
    if (app == NULL)
        return;
    if (app->screen) {
        screen_free(app->screen);
        app->screen = NULL;
    }
}

static void menu_continue_game(void *ctx) {
    App *app = (App *)ctx;
    if (app->game.status == GAME_STATUS_PAUSED) {
        app->game.status = GAME_STATUS_RUNNING;
    }
    app->current_screen = SCREEN_GAME;
}

static void menu_quit(void *ctx) {
    App *app = (App *)ctx;
    app->running = false;
}

static void menu_new_game(void *ctx) {
    App *app = (App *)ctx;
    game_reset(&app->game);
    level_spawn(&app->game, &LEVELS[0]);
    app->current_screen = SCREEN_GAME;
}

static void process_input(App *app, Event *ev) {
    if (ev->type != EVENT_KEY)
        return;

    // Global keys (work on any screen)
    if (key_is_char(ev, 'q') || key_is_char(ev, 'Q')) {
        app->running = false;
        return;
    }

    switch (app->current_screen) {
    case SCREEN_GAME:
        // M/m - open menu
        if (key_is_char(ev, 'm') || key_is_char(ev, 'M')) {
            app->game.status = GAME_STATUS_PAUSED;
            app->current_screen = SCREEN_MENU;
            return;
        }
        // Forward to game input handler
        game_process_input(&app->game, ev);
        break;

    case SCREEN_MENU:
        if (key_is(ev, KEY_UP)) {
            menu_up(&app->menu);
        } else if (key_is(ev, KEY_DOWN)) {
            menu_down(&app->menu);
        } else if (key_is(ev, KEY_ENTER)) {
            menu_select(&app->menu);
        }
        break;

    case SCREEN_GAME_OVER:
        if (key_is(ev, KEY_UP)) {
            menu_up(&app->game_over_menu);
        } else if (key_is(ev, KEY_DOWN)) {
            menu_down(&app->game_over_menu);
        } else if (key_is(ev, KEY_ENTER)) {
            menu_select(&app->game_over_menu);
        }
        break;

    case SCREEN_WIN:
        if (key_is(ev, KEY_UP)) {
            menu_up(&app->win_menu);
        } else if (key_is(ev, KEY_DOWN)) {
            menu_down(&app->win_menu);
        } else if (key_is(ev, KEY_ENTER)) {
            menu_select(&app->win_menu);
        }
        break;
    }
}

static void *event_loop(void *ctx) {
    App *app = (App *)ctx;

    while (app->running) {
        Event ev;

        pthread_mutex_lock(&app->mutex);
        bool got_event = input_poll(&app->input, &ev);
        pthread_mutex_unlock(&app->mutex);

        if (got_event) {
            pthread_mutex_lock(&app->mutex);
            process_input(app, &ev);
            app_render(app); // Immediate feedback after input
            pthread_mutex_unlock(&app->mutex);
        }

        // Small sleep to avoid busy-waiting
        struct timespec ts = {0, 500000}; // 0.5ms
        nanosleep(&ts, NULL);
    }

    return NULL;
}

static void app_render(App *app) {
    screen_clear(app->screen);

    switch (app->current_screen) {
    case SCREEN_GAME:
        frame_draw_all(app->frame, app->screen);
        break;
    case SCREEN_MENU:
        menu_draw(&app->menu, app->screen);
        break;
    case SCREEN_GAME_OVER:
        menu_draw(&app->game_over_menu, app->screen);
        break;
    case SCREEN_WIN:
        menu_draw(&app->win_menu, app->screen);
        break;
    }

    screen_flush(app->screen);
}

int main(void) {
    App app = {0};
    app.running = true;
    app.current_screen = SCREEN_GAME;
    pthread_mutex_init(&app.mutex, NULL);

    app.screen = screen_new();
    if (!app.screen) {
        fprintf(stderr, "Failed to initialize screen\n");
        return 1;
    }

    input_buffer_init(&app.input);

    app.frame = frame_new(app.screen->width, app.screen->height, 0, 0);
    Frame *(*split)[2] =
        frame_split(app.frame, (app.frame->h - 4), FRAME_SPLIT_HORIZONTAL);
    Frame **split1 = *split;
    split =
        frame_split(split1[0], (split1[0]->w * 2 / 3), FRAME_SPLIT_VERTICAL);
    Frame **split2 = *split;

    // Controls panel at bottom
    Panel controls = {0};
    panel_empty_line(&controls);
    panel_add_item(&controls,
                   (PanelItem){
                       .ctx = " Controls:    [<] / [a] Left    [>] / [d] Right "
                              "   [space] Shoot    [q] Quit    [m] Menu",
                       .draw = panel_item_text_draw,
                   });
    frame_add_widget(split1[1], panel_as_widget(&controls));

    game_init(&app.game, 11, 11, 4);
    level_spawn(&app.game, &LEVELS[0]);

    frame_add_widget(split2[0], (Widget){
                                  .ctx = &app.game,
                                  .draw = game_draw,
                                  .free_item = (WidgetFreeFn)game_free,
                              });

    app.menu.selected = -1;
    app.menu.at = 2;

    Entry menu_entries[] = {
        {.name = "Menu:",
         .selectable = false,
         .ctx = NULL,
         .on_selected = NULL},
        {.name = NULL, .selectable = false, .ctx = NULL, .on_selected = NULL},
        {.name = "Continue",
         .selectable = true,
         .ctx = &app,
         .on_selected = menu_continue_game},
        {.name = "New Game",
         .selectable = true,
         .ctx = &app,
         .on_selected = menu_new_game},
        {.name = "Quit",
         .selectable = true,
         .ctx = &app,
         .on_selected = menu_quit},
    };

    for (usize i = 0; i < ARRAY_LEN(menu_entries); i++) {
        menu_add_entry(&app.menu, menu_entries[i]);
    }

    app.game_over_menu.selected = -1;
    app.game_over_menu.at = 2;

    Entry game_over_entries[] = {
        {.name = "Game Over!",
         .selectable = false,
         .ctx = NULL,
         .on_selected = NULL},
        {.name = NULL, .selectable = false, .ctx = NULL, .on_selected = NULL},
        {.name = "New Game",
         .selectable = true,
         .ctx = &app,
         .on_selected = menu_new_game},
        {.name = "Quit",
         .selectable = true,
         .ctx = &app,
         .on_selected = menu_quit},
    };

    for (usize i = 0; i < ARRAY_LEN(game_over_entries); i++) {
        menu_add_entry(&app.game_over_menu, game_over_entries[i]);
    }

    app.win_menu.selected = -1;
    app.win_menu.at = 2;

    Entry win_entries[] = {
        // clang-format off
        {.name = "You Win!", .selectable = false, .ctx = NULL, .on_selected = NULL},
        {.name = NULL, .selectable = false, .ctx = NULL, .on_selected = NULL},
        {.name = "New Game", .selectable = true, .ctx = &app, .on_selected = menu_new_game},
        {.name = "Quit", .selectable = true, .ctx = &app, .on_selected = menu_quit},
        // clang-format on
    };

    for (usize i = 0; i < ARRAY_LEN(win_entries); i++) {
        menu_add_entry(&app.win_menu, win_entries[i]);
    }

    // Stats panel on the right
    Panel stats = {0};
    Value values[] = {
        // clang-format off
        {.label = " Level", .ctx = &app.game.level, .stringify = value_stringify_int},
        {.label = " Score", .ctx = &app.game.score, .stringify = value_stringify_int},
        {.label = " Lives", .ctx = &((Player *)app.game.player.ctx)->health, .stringify = value_stringify_int},
        {0},
        {.label = " Kills", .ctx = &app.game.kills, .stringify = value_stringify_int},
        {.label = " Accuracy", .ctx = &app.game.accuracy_percent, .stringify = value_stringify_percent},
        {.label = " Time", .ctx = &app.game.time_seconds, .stringify = value_stringify_time},
        {0},
        {.label = " # Bullets on screen", .ctx = &app.game.bullets.count, .stringify = value_stringify_int},
        {.label = " # Enemies remaining", .ctx = &app.game.enemies.count, .stringify = value_stringify_int},
        // clang-format on
    };

    for (usize i = 0; i < ARRAY_LEN(values); i++) {
        if (values[i].label) {
            panel_add_item(&stats, value_as_panel_item(&values[i]));
        } else {
            panel_empty_line(&stats);
        }
    }

    frame_add_widget(split2[1], panel_as_widget(&stats));

    // Initial render
    app_render(&app);

    // Start input thread
    pthread_t input_thread;
    pthread_create(&input_thread, NULL, event_loop, &app);

    i64 sleep_time = US_PER_SEC / app.game.tps;

    while (app.running) {
        pthread_mutex_lock(&app.mutex);

        if (app.game.status == GAME_STATUS_RUNNING) {
            game_update(&app.game, sleep_time);
            game_remove_dead_entities(&app.game);

            // Check level completion
            if (app.game.enemies.count == 0 && app.game.player.alive) {
                app.game.level++;
                if (app.game.level > LEVEL_COUNT) {
                    app.game.status = GAME_STATUS_FINISHED;
                    app.current_screen = SCREEN_WIN;
                } else {
                    level_spawn(&app.game, &LEVELS[app.game.level - 1]);
                }
            }

            if (app.game.status == GAME_STATUS_GAME_OVER) {
                app.current_screen = SCREEN_GAME_OVER;
            }
        }

        app_render(&app);
        pthread_mutex_unlock(&app.mutex);

        struct timespec ts;
        ts.tv_sec = (time_t)(sleep_time / US_PER_SEC);
        ts.tv_nsec = (long)((sleep_time % US_PER_SEC) * 1000LL);
        nanosleep(&ts, NULL);
    }

    app.running = false;
    pthread_join(input_thread, NULL);

    frame_free(app.frame);
    menu_free(&app.menu);
    menu_free(&app.game_over_menu);
    menu_free(&app.win_menu);
    screen_free(app.screen);
    pthread_mutex_destroy(&app.mutex);

    return 0;
}
