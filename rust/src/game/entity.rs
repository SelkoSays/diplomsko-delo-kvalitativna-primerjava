//! Entity system using Rust enums for polymorphic behavior

use crate::tui::{cell::Color, screen::Screen};

/// Entity color (maps to screen Color)
#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
#[allow(unused)]
pub enum EntityColor {
    #[default]
    None,
    Red,
    Green,
    Blue,
    Cyan,
    Yellow,
}

impl EntityColor {
    pub fn to_color(self) -> Color {
        match self {
            EntityColor::None => Color::WHITE,
            EntityColor::Red => Color::RED,
            EntityColor::Green => Color::GREEN,
            EntityColor::Blue => Color::BLUE,
            EntityColor::Cyan => Color::CYAN,
            EntityColor::Yellow => Color::YELLOW,
        }
    }
}

/// Entity owner type (for bullets)
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Owner {
    Player,
    Enemy,
}

/// Common entity data
#[derive(Clone, Debug)]
pub struct EntityBase {
    pub x: i32,
    pub y: i32,
    pub alive: bool,
    pub shape: String,
    pub color: EntityColor,
}

impl EntityBase {
    pub fn new(x: i32, y: i32, shape: &str) -> Self {
        EntityBase {
            x,
            y,
            alive: true,
            shape: shape.to_string(),
            color: EntityColor::None,
        }
    }

    pub fn draw(&self, screen: &mut Screen, x: usize, y: usize) {
        if !self.alive {
            return;
        }
        screen.put_char(x, y, &self.shape);
        screen.set_fg_color(x, y, self.color.to_color());
    }
}

/// Player-specific data
#[derive(Clone, Debug)]
pub struct PlayerData {
    pub health: i32,
    pub damage: i32,
    pub cooldown: i32,
    pub ticks: i32,
    pub damaged: bool,
}

/// Enemy-specific data
#[derive(Clone, Debug)]
pub struct EnemyData {
    pub health: i32,
    pub score: i32,
    pub fire_freq: i32,
    pub last_fired: i32,
    pub damage: i32,
}

/// Bullet-specific data
#[derive(Clone, Debug)]
pub struct BulletData {
    pub owner: Owner,
    pub damage: i32,
    pub move_freq: i32,
    pub last_moved: i32,
}

/// Entity enum - polymorphic entity type
#[derive(Clone, Debug)]
pub enum Entity {
    Player { base: EntityBase, data: PlayerData },
    Enemy { base: EntityBase, data: EnemyData },
    Bullet { base: EntityBase, data: BulletData },
}

#[allow(unused)]
impl Entity {
    // Factory methods
    pub fn new_player(x: i32, y: i32, health: i32, damage: i32, cooldown: i32) -> Self {
        let mut base = EntityBase::new(x, y, "A");
        base.color = EntityColor::Cyan;
        Entity::Player {
            base,
            data: PlayerData {
                health,
                damage,
                cooldown,
                ticks: cooldown,
                damaged: false,
            },
        }
    }

    pub fn new_enemy(x: i32, y: i32, health: i32, score: i32, fire_freq: i32, damage: i32) -> Self {
        Entity::Enemy {
            base: EntityBase::new(x, y, "V"),
            data: EnemyData {
                health,
                score,
                fire_freq,
                last_fired: 0,
                damage,
            },
        }
    }

    pub fn new_bullet(x: i32, y: i32, damage: i32, owner: Owner) -> Self {
        let shape = if owner == Owner::Enemy { "|" } else { "0" };
        Entity::Bullet {
            base: EntityBase::new(x, y, shape),
            data: BulletData {
                owner,
                damage,
                move_freq: 1,
                last_moved: 0,
            },
        }
    }

    // Common accessors
    pub fn base(&self) -> &EntityBase {
        match self {
            Entity::Player { base, .. }
            | Entity::Enemy { base, .. }
            | Entity::Bullet { base, .. } => base,
        }
    }

    pub fn base_mut(&mut self) -> &mut EntityBase {
        match self {
            Entity::Player { base, .. }
            | Entity::Enemy { base, .. }
            | Entity::Bullet { base, .. } => base,
        }
    }

    pub fn x(&self) -> i32 {
        self.base().x
    }
    pub fn y(&self) -> i32 {
        self.base().y
    }
    pub fn is_alive(&self) -> bool {
        self.base().alive
    }

    pub fn set_x(&mut self, x: i32) {
        self.base_mut().x = x;
    }
    pub fn set_y(&mut self, y: i32) {
        self.base_mut().y = y;
    }
    pub fn kill(&mut self) {
        self.base_mut().alive = false;
    }

    pub fn collides_with(&self, other: &Entity) -> bool {
        self.x() == other.x() && self.y() == other.y()
    }

    pub fn draw(&self, screen: &mut Screen, x: usize, y: usize) {
        self.base().draw(screen, x, y);
    }

    /// Apply damage, returns score gained (for enemies)
    pub fn damage(&mut self, amount: i32) -> i32 {
        match self {
            Entity::Player { base, data } => {
                if data.health <= amount {
                    data.health = 0;
                    base.alive = false;
                } else {
                    data.health -= amount;
                }
                data.damaged = true;
                base.color = EntityColor::None;
                0
            }
            Entity::Enemy { base, data } => {
                if data.health <= amount {
                    data.health = 0;
                    base.alive = false;
                    data.score
                } else {
                    data.health -= amount;
                    0
                }
            }
            Entity::Bullet { .. } => 0,
        }
    }

    /// Get player data if this is a player
    pub fn as_player(&self) -> Option<&PlayerData> {
        match self {
            Entity::Player { data, .. } => Some(data),
            _ => None,
        }
    }

    pub fn as_player_mut(&mut self) -> Option<&mut PlayerData> {
        match self {
            Entity::Player { data, .. } => Some(data),
            _ => None,
        }
    }

    pub fn as_enemy(&self) -> Option<&EnemyData> {
        match self {
            Entity::Enemy { data, .. } => Some(data),
            _ => None,
        }
    }

    pub fn as_enemy_mut(&mut self) -> Option<&mut EnemyData> {
        match self {
            Entity::Enemy { data, .. } => Some(data),
            _ => None,
        }
    }

    #[allow(dead_code)]
    pub fn as_bullet(&self) -> Option<&BulletData> {
        match self {
            Entity::Bullet { data, .. } => Some(data),
            _ => None,
        }
    }

    pub fn as_bullet_mut(&mut self) -> Option<&mut BulletData> {
        match self {
            Entity::Bullet { data, .. } => Some(data),
            _ => None,
        }
    }
}
