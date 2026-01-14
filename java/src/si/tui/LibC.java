package si.tui;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

public interface LibC extends Library {
    LibC INSTANCE = Native.load("c", LibC.class);

    int STDIN_FILENO = 0;
    int STDOUT_FILENO = 1;

    // c_lflag bits (Linux octal values converted to hex)
    int ECHO = 0x0008;      // octal 0000010
    int ICANON = 0x0002;    // octal 0000002
    int ISIG = 0x0001;      // octal 0000001
    int IEXTEN = 0x8000;    // octal 0100000

    // c_iflag bits
    int IXON = 0x0400;      // octal 0002000
    int ICRNL = 0x0100;     // octal 0000400
    int BRKINT = 0x0002;    // octal 0000002
    int INPCK = 0x0010;     // octal 0000020
    int ISTRIP = 0x0020;    // octal 0000040

    // c_oflag bits
    int OPOST = 0x0001;     // octal 0000001

    // c_cflag bits
    int CS8 = 0x0030;       // octal 0000060

    // tcsetattr actions
    int TCSAFLUSH = 2;

    // ioctl for window size
    int TIOCGWINSZ = 0x5413;

    @Structure.FieldOrder({"c_iflag", "c_oflag", "c_cflag", "c_lflag", "c_line", "c_cc", "c_ispeed", "c_ospeed"})
    class Termios extends Structure {
        public int c_iflag;
        public int c_oflag;
        public int c_cflag;
        public int c_lflag;
        public byte c_line;
        public byte[] c_cc = new byte[32];
        public int c_ispeed;
        public int c_ospeed;
    }

    @Structure.FieldOrder({"ws_row", "ws_col", "ws_xpixel", "ws_ypixel"})
    class Winsize extends Structure {
        public short ws_row;
        public short ws_col;
        public short ws_xpixel;
        public short ws_ypixel;
    }

    int tcgetattr(int fd, Termios termios);
    int tcsetattr(int fd, int actions, Termios termios);
    int ioctl(int fd, int request, Winsize winsize);
}
