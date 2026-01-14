//! Terminal RAII wrapper using libc termios

use super::escape;
use std::io::{self, Write};
use std::mem::MaybeUninit;

pub struct Terminal {
    orig_termios: libc::termios,
    initialized: bool,
}

impl Terminal {
    pub fn new() -> io::Result<Self> {
        let mut orig_termios = MaybeUninit::<libc::termios>::uninit();

        // Get original terminal settings
        if unsafe { libc::tcgetattr(libc::STDIN_FILENO, orig_termios.as_mut_ptr()) } != 0 {
            return Err(io::Error::last_os_error());
        }

        let orig_termios = unsafe { orig_termios.assume_init() };
        let mut raw = orig_termios;

        // Set raw mode flags (matching C/C++/Java implementations)
        // Disable echo, canonical mode, signals, extended input processing
        raw.c_lflag &= !(libc::ECHO | libc::ICANON | libc::ISIG | libc::IEXTEN);

        // Disable XON/XOFF flow control, CR to NL mapping
        raw.c_iflag &= !(libc::IXON | libc::ICRNL);

        // Disable output processing
        raw.c_oflag &= !libc::OPOST;

        // Set 8-bit chars
        raw.c_cflag |= libc::CS8;

        // Non-blocking read: VMIN = 0, VTIME = 0
        raw.c_cc[libc::VMIN] = 0;
        raw.c_cc[libc::VTIME] = 0;

        if unsafe { libc::tcsetattr(libc::STDIN_FILENO, libc::TCSAFLUSH, &raw) } != 0 {
            return Err(io::Error::last_os_error());
        }

        // Setup alternate screen, cursor, mouse
        print!(
            "{}{}{}{}",
            escape::ALT_SCREEN_ON,
            escape::CURSOR_HIDE,
            escape::MOUSE_ON,
            escape::CLEAR_SCREEN
        );
        io::stdout().flush()?;

        Ok(Terminal {
            orig_termios,
            initialized: true,
        })
    }

    pub fn size() -> io::Result<(u16, u16)> {
        let mut ws = MaybeUninit::<libc::winsize>::uninit();
        if unsafe { libc::ioctl(libc::STDOUT_FILENO, libc::TIOCGWINSZ, ws.as_mut_ptr()) } != 0 {
            return Ok((80, 24)); // Fallback
        }
        let ws = unsafe { ws.assume_init() };
        Ok((ws.ws_col, ws.ws_row))
    }
}

impl Drop for Terminal {
    fn drop(&mut self) {
        if !self.initialized {
            return;
        }

        print!(
            "{}{}{}{}",
            escape::RESET_ATTRS,
            escape::MOUSE_OFF,
            escape::CURSOR_SHOW,
            escape::ALT_SCREEN_OFF
        );
        let _ = io::stdout().flush();

        unsafe { libc::tcsetattr(libc::STDIN_FILENO, libc::TCSAFLUSH, &self.orig_termios) };
    }
}
