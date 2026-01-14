//! Common type aliases and utilities

use std::error::Error;
use std::time::{SystemTime, UNIX_EPOCH};

/// Standard result type for the application
pub type Result<T> = std::result::Result<T, Box<dyn Error + Send + Sync>>;

/// Microseconds per second
pub const US_PER_SEC: i64 = 1_000_000;

#[allow(unused)]
/// Get current time in microseconds
pub fn time_us() -> i64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_micros() as i64)
        .unwrap_or(0)
}
