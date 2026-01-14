//! Game state container

use crate::common::US_PER_SEC;
use crate::game::entity::{Entity, EntityColor, Owner};
use crate::input::{self, Event, KeyCode};
use crate::tui::screen::Screen;
use crate::ui::widget::BBox;

/// Game status
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum GameStatus {
    Running,
    Paused,
    Finished,
    GameOver,
}

/// Game bounds
#[derive(Clone, Copy, Debug)]
pub struct Bounds {
    pub w: i32,
    pub h: i32,
}

/// Game state
pub struct Game {
    bounds: Bounds,
    level: i32,
    score: i32,
    status: GameStatus,

    player: Option<Entity>,
    enemies: Vec<Entity>,
    bullets: Vec<Entity>,

    tps: i64,

    // Statistics
    shots_fired: i32,
    shots_hit: i32,
    kills: i32,
    elapsed_time: i64,
}

#[allow(unused)]
impl Game {
    pub fn new(w: i32, h: i32, tps: i64) -> Self {
        let mut game = Game {
            bounds: Bounds { w, h },
            level: 1,
            score: 0,
            status: GameStatus::Running,
            player: None,
            enemies: Vec::new(),
            bullets: Vec::new(),
            tps,
            shots_fired: 0,
            shots_hit: 0,
            kills: 0,
            elapsed_time: 0,
        };

        game.spawn_player((w - 1) / 2, h - 1, 5, 1, 2);
        game
    }

    pub fn bounds(&self) -> Bounds {
        self.bounds
    }
    pub fn level(&self) -> i32 {
        self.level
    }
    pub fn score(&self) -> i32 {
        self.score
    }
    pub fn status(&self) -> GameStatus {
        self.status
    }
    pub fn tps(&self) -> i64 {
        self.tps
    }
    #[allow(dead_code)]
    pub fn shots_fired(&self) -> i32 {
        self.shots_fired
    }
    #[allow(dead_code)]
    pub fn shots_hit(&self) -> i32 {
        self.shots_hit
    }
    pub fn kills(&self) -> i32 {
        self.kills
    }
    pub fn time_seconds(&self) -> i32 {
        (self.elapsed_time / US_PER_SEC) as i32
    }
    pub fn accuracy_percent(&self) -> i32 {
        if self.shots_fired > 0 {
            (self.shots_hit * 100) / self.shots_fired
        } else {
            0
        }
    }
    pub fn bullet_count(&self) -> usize {
        self.bullets.len()
    }
    pub fn enemy_count(&self) -> usize {
        self.enemies.len()
    }

    pub fn player(&self) -> Option<&Entity> {
        self.player.as_ref()
    }
    pub fn player_health(&self) -> i32 {
        self.player
            .as_ref()
            .and_then(|p| p.as_player())
            .map(|d| d.health)
            .unwrap_or(0)
    }

    pub fn set_status(&mut self, status: GameStatus) {
        self.status = status;
    }
    pub fn increment_level(&mut self) {
        self.level += 1;
    }

    pub fn spawn_player(&mut self, x: i32, y: i32, health: i32, damage: i32, cooldown: i32) {
        self.player = Some(Entity::new_player(x, y, health, damage, cooldown));
    }

    pub fn spawn_enemy(
        &mut self,
        x: i32,
        y: i32,
        health: i32,
        score: i32,
        fire_freq: i32,
        damage: i32,
    ) {
        self.enemies
            .push(Entity::new_enemy(x, y, health, score, fire_freq, damage));
    }

    pub fn spawn_bullet(&mut self, x: i32, y: i32, damage: i32, owner: Owner) {
        self.bullets.push(Entity::new_bullet(x, y, damage, owner));
    }

    pub fn reset(&mut self) {
        self.enemies.clear();
        self.bullets.clear();

        if let Some(player) = &mut self.player {
            if let Entity::Player { data, .. } = player {
                data.health = 5;
                data.ticks = data.cooldown;
                data.damaged = false;
            }
            let base = player.base_mut();
            base.alive = true;
            base.x = (self.bounds.w - 1) / 2;
            base.y = self.bounds.h - 1;
            base.color = EntityColor::Cyan;
        }

        self.level = 1;
        self.score = 0;
        self.status = GameStatus::Running;
        self.shots_fired = 0;
        self.shots_hit = 0;
        self.kills = 0;
        self.elapsed_time = 0;
    }

    pub fn update(&mut self, delta_us: i64) {
        self.elapsed_time += delta_us;

        self.update_player();

        for i in 0..self.enemies.len() {
            self.update_enemy(i);
        }

        let player_pos = self
            .player
            .as_ref()
            .filter(|p| p.is_alive())
            .map(|p| (p.x(), p.y()));

        let enemy_positions: Vec<(i32, i32)> =
            self.enemies.iter().map(|e| (e.x(), e.y())).collect();

        for i in 0..self.bullets.len() {
            self.update_bullet(i);
        }
    }

    fn update_player(&mut self) {
        if let Some(Entity::Player { base, data }) = &mut self.player {
            data.ticks += 1;
            if data.damaged {
                data.damaged = false;
            } else {
                base.color = EntityColor::Cyan;
            }
        }
    }

    fn update_enemy(&mut self, i: usize) {
        let mut should_fire = false;
        let (mut x, mut y, mut dmg) = (0, 0, 0);
        if let Entity::Enemy { base, data } = &mut self.enemies[i] {
            base.color = EntityColor::None;
            data.last_fired += 1;

            if data.last_fired >= (data.fire_freq - 1) {
                base.color = EntityColor::Red;
            }

            should_fire = data.last_fired >= data.fire_freq;
            x = base.x;
            y = base.y;
            dmg = data.damage;

            if should_fire {
                data.last_fired = 0;
            }
        }

        if should_fire {
            self.spawn_bullet(x, y + 1, dmg, Owner::Enemy);
        }
    }

    fn update_bullet(&mut self, i: usize) {
        let bounds = self.bounds;
        let player_pos = self
            .player
            .as_ref()
            .filter(|p| p.is_alive())
            .map(|p| (p.x(), p.y()));

        if let Entity::Bullet { base, data } = &mut self.bullets[i] {
            data.last_moved += 1;

            if data.last_moved < data.move_freq {
                return;
            }
            data.last_moved = 0;

            if data.owner == Owner::Enemy {
                if let Some((px, py)) = player_pos {
                    if base.x == px && base.y == py {
                        self.player.as_mut().map(|p| {
                            p.damage(data.damage);
                        });
                        base.alive = false;
                        return;
                    }
                }

                // Move down
                if base.y + 1 >= bounds.h {
                    base.alive = false;
                } else {
                    base.y += 1;
                }
            } else {
                for i in 0..self.enemies.len() {
                    if base.x == self.enemies[i].x() && base.y == self.enemies[i].y() {
                        self.enemies[i].damage(data.damage);
                        base.alive = false;
                        return;
                    }
                }

                // Move up
                if base.y - 1 < 0 {
                    base.alive = false;
                } else {
                    base.y -= 1;
                }
            }
        }
    }

    pub fn remove_dead_entities(&mut self) {
        self.bullets.retain(|b| b.is_alive());
        self.enemies.retain(|e| e.is_alive());

        if let Some(player) = &self.player {
            if !player.is_alive() {
                self.status = GameStatus::GameOver;
            }
        }
    }

    pub fn process_input(&mut self, event: &Event) {
        let Event::Key(key) = event else { return };

        let Some(player) = &mut self.player else {
            return;
        };

        let px = player.x();
        let py = player.y();

        if input::is_char(key, 'a') || input::is_key(key, KeyCode::Left) {
            if px > 0 {
                player.set_x(px - 1);
            }
        } else if input::is_char(key, 'd') || input::is_key(key, KeyCode::Right) {
            if px < self.bounds.w - 1 {
                player.set_x(px + 1);
            }
        } else if input::is_char(key, ' ') {
            if let Entity::Player { data, .. } = player {
                if data.ticks >= data.cooldown {
                    let damage = data.damage;
                    data.ticks = 0;
                    self.shots_fired += 1;
                    self.bullets
                        .push(Entity::new_bullet(px, py - 1, damage, Owner::Player));
                }
            }
        }
    }

    pub fn draw(&self, screen: &mut Screen, bbox: BBox) {
        let x_pad = (bbox.w as i32 / self.bounds.w) / 2;
        let y_pad = (bbox.h as i32 / self.bounds.h) / 2;

        for bullet in &self.bullets {
            let xx = (bullet.x() * bbox.w as i32 / self.bounds.w + bbox.x as i32 + x_pad) as usize;
            let yy = (bullet.y() * bbox.h as i32 / self.bounds.h + bbox.y as i32 + y_pad) as usize;
            bullet.draw(screen, xx, yy);
        }

        for enemy in &self.enemies {
            let xx = (enemy.x() * bbox.w as i32 / self.bounds.w + bbox.x as i32 + x_pad) as usize;
            let yy = (enemy.y() * bbox.h as i32 / self.bounds.h + bbox.y as i32 + y_pad) as usize;
            enemy.draw(screen, xx, yy);
        }

        if let Some(player) = &self.player {
            if player.is_alive() {
                let xx =
                    (player.x() * bbox.w as i32 / self.bounds.w + bbox.x as i32 + x_pad) as usize;
                let yy =
                    (player.y() * bbox.h as i32 / self.bounds.h + bbox.y as i32 + y_pad) as usize;
                player.draw(screen, xx, yy);
            }
        }
    }
}
