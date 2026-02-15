package si.tui;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;

public final class Terminal implements AutoCloseable {
    private final String savedState;
    private boolean initialized = false;
    private int width = 80;
    private int height = 24;

    public Terminal() {
        savedState = stty("-g").trim();

        stty("raw", "-echo", "-icanon", "-isig", "-iexten",
             "-ixon", "-icrnl", "-brkint", "-inpck", "-istrip",
             "-opost", "cs8", "min", "0", "time", "0");

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
        String output = stty("size").trim();
        String[] parts = output.split("\\s+");
        if (parts.length == 2) {
            height = Integer.parseInt(parts[0]);
            width = Integer.parseInt(parts[1]);
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

        try {
            stty(savedState);
        } catch (RuntimeException e) {
            // Retry by writing saved state to stty's stdin
            try {
                ProcessBuilder pb = new ProcessBuilder("stty");
                pb.redirectInput(new File("/dev/tty"));
                pb.redirectErrorStream(true);
                Process p = pb.start();
                p.getOutputStream().write(savedState.getBytes());
                p.getOutputStream().close();
                p.waitFor();
            } catch (Exception ex) {
                // Best effort restoration
            }
        }

        initialized = false;
    }

    private static String stty(String... args) {
        try {
            String[] cmd = new String[args.length + 1];
            cmd[0] = "stty";
            System.arraycopy(args, 0, cmd, 1, args.length);

            ProcessBuilder pb = new ProcessBuilder(cmd);
            pb.redirectInput(new File("/dev/tty"));
            pb.redirectErrorStream(true);
            Process p = pb.start();

            String output;
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(p.getInputStream()))) {
                output = reader.lines()
                        .reduce("", (a, b) -> a + b + "\n");
            }

            int exitCode = p.waitFor();
            if (exitCode != 0) {
                throw new RuntimeException(
                        "stty exited with code " + exitCode + ": " + output);
            }
            return output;
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("Failed to run stty", e);
        }
    }
}
