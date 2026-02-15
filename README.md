# Program implementiran za diplomsko delo

Implementacija igre v štirih programskih jezikih: C, C++, Rust in Java.

## Podprte platforme

- Linux
- macOS

Vse implementacije za nastavitev terminala uporabljajo POSIX orodja (`stty`, termios), zato delujejo na obeh platformah.

## Zahteve

### C

- Prevajalnik `clang` (ali `gcc`)
- Knjižnice: `libpthread`
- Standard: C17

### C++

- Prevajalnik `clang++` (ali `g++`)
- Standard: C++17

### Rust

- `cargo` in `rustc`

### Java

- Preverjeno na JDK 21

## Prevajanje

### Vse verzije naenkrat

```bash
./build.sh all
```

### Posamezna verzija

```bash
./build.sh c      # C verzija
./build.sh cpp    # C++ verzija
./build.sh rust   # Rust verzija
./build.sh java   # Java verzija
```

## Zagon

```bash
./run.sh c        # Zaženi C verzijo
./run.sh cpp      # Zaženi C++ verzijo
./run.sh rust     # Zaženi Rust verzijo
./run.sh java     # Zaženi Java verzijo
```

## Kontrole

| Tipka       | Akcija       |
| ----------- | ------------ |
| `a` / `←`   | Premik levo  |
| `d` / `→`   | Premik desno |
| `presledek` | Streljanje   |
| `m`         | Meni         |
| `q`         | Izhod        |

## Struktura projekta

```sh
./
|-- c/           # C implementacija
|-- cpp/         # C++ implementacija
|-- rust/        # Rust implementacija
|-- java/        # Java implementacija
|-- build.sh     # Skripta za prevajanje
|-- run.sh       # Skripta za zagon
|-- LICENSE      # Licenca
\-- README.md
```
