package si.tui;

public final class Terminal implements AutoCloseable {
    private final LibC.Termios originalTermios;
    private boolean initialized = false;
    private int width = 80;
    private int height = 24;

    public Terminal() {
        originalTermios = new LibC.Termios();

        if (LibC.INSTANCE.tcgetattr(LibC.STDIN_FILENO, originalTermios) != 0) {
            throw new RuntimeException("Failed to get terminal attributes");
        }

        LibC.Termios raw = new LibC.Termios();
        raw.c_iflag = originalTermios.c_iflag;
        raw.c_oflag = originalTermios.c_oflag;
        raw.c_cflag = originalTermios.c_cflag;
        raw.c_lflag = originalTermios.c_lflag;
        raw.c_line = originalTermios.c_line;
        System.arraycopy(originalTermios.c_cc, 0, raw.c_cc, 0, originalTermios.c_cc.length);
        raw.c_ispeed = originalTermios.c_ispeed;
        raw.c_ospeed = originalTermios.c_ospeed;

        // Disable echo, canonical mode, signals, extended input processing
        raw.c_lflag &= ~(LibC.ECHO | LibC.ICANON | LibC.ISIG | LibC.IEXTEN);

        // Disable XON/XOFF flow control, CR to NL mapping
        raw.c_iflag &= ~(LibC.IXON | LibC.ICRNL | LibC.BRKINT | LibC.INPCK | LibC.ISTRIP);

        // Disable output processing
        raw.c_oflag &= ~(LibC.OPOST);

        // Set 8-bit chars
        raw.c_cflag |= LibC.CS8;

        // Non-blocking read: VMIN = 0, VTIME = 0
        raw.c_cc[6] = 0;  // VMIN
        raw.c_cc[5] = 0;  // VTIME

        if (LibC.INSTANCE.tcsetattr(LibC.STDIN_FILENO, LibC.TCSAFLUSH, raw) != 0) {
            throw new RuntimeException("Failed to set terminal to raw mode");
        }

        updateSize();

        System.out.print(Escape.ALT_SCREEN_ON);
        System.out.print(Escape.CURSOR_HIDE);
        System.out.print(Escape.MOUSE_ON);
        System.out.print(Escape.CLEAR_SCREEN);
        System.out.flush();

        initialized = true;

        Runtime.getRuntime().addShutdownHook(new Thread(this::close));
    }

    public void updateSize() {
        LibC.Winsize ws = new LibC.Winsize();
        if (LibC.INSTANCE.ioctl(LibC.STDOUT_FILENO, LibC.TIOCGWINSZ, ws) == 0) {
            width = ws.ws_col;
            height = ws.ws_row;
        }
    }

    public int width() {
        return width;
    }

    public int height() {
        return height;
    }

    @Override
    public void close() {
        if (!initialized) {
            return;
        }

        System.out.print(Escape.RESET_ATTRS);
        System.out.print(Escape.MOUSE_OFF);
        System.out.print(Escape.CURSOR_SHOW);
        System.out.print(Escape.ALT_SCREEN_OFF);
        System.out.flush();

        LibC.INSTANCE.tcsetattr(LibC.STDIN_FILENO, LibC.TCSAFLUSH, originalTermios);

        initialized = false;
    }
}
