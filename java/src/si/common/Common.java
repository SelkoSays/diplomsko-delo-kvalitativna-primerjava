package si.common;

public final class Common {
    public static final long US_PER_SEC = 1_000_000L;

    public static long timeUs() {
        return System.nanoTime() / 1000L;
    }

    private Common() {
    }
}
