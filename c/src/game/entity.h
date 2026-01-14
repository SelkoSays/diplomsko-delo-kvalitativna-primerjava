#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include "../common.h"
#include "../tui/screen.h"


typedef enum EntityColor {
    ECOLOR_NONE,
    ECOLOR_RED,
    ECOLOR_GREEN,
    ECOLOR_BLUE,
    ECOLOR_CYAN,
    ECOLOR_YELLOW,
} EntityColor;

// Convert entity color to screen Color
Color ecolor_to_color(EntityColor ecolor);


typedef enum EntityType {
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_BULLET,
} EntityType;


typedef struct Entity {
    int x, y;
    bool alive;
    char *shape;
    EntityType type;
    int (*damage)(struct Entity *, int);
    void (*update)(struct Entity *);
    EntityColor color;
    void *ctx;  // context (Player*, Enemy*, Bullet*)
} Entity;


// Call entity's damage function
int entity_damage(Entity *e, int amount);

// Call entity's update function
void entity_update(Entity *e);

// Check if two entities collide
bool entity_collision(const Entity *e1, const Entity *e2);

// Draw entity at screen position
void entity_draw(Entity *e, Screen *s, int x, int y);

typedef struct Player {
    int health;
    int damage;
    int cooldown;  // ticks between shots
    int ticks;     // ticks since last fired
    bool damaged;
} Player;

// Forward declaration
struct Game;

typedef struct Enemy {
    int health;
    int score;
    int fire_freq;   // ticks between shots
    int last_fired;
    int damage;
    struct Game *game;  // reference to game for spawning bullets
} Enemy;

typedef struct Bullet {
    EntityType owner;
    int damage;
    int move_freq;   // ticks between moves
    int last_moved;
    struct Game *game;  // reference to game for collision checks
} Bullet;

#endif // GAME_ENTITY_H
