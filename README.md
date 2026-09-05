# Term_File_Search

A terminal file explorer built on raw ANSI escape codes and termios, with vim
style navigation. No ncurses. Written to understand how TUIs actually work at
the syscall level.

Binary target is `Cexp`.

## Build

```bash
cmake -S . -B out/Debug
cmake --build out/Debug
./out/Debug/Cexp

```

Requires C++17 for std::filesystem. Optionally takes a path argument to open
into a starting folder.

Keys

┌─────┬───────────────────────────────────┐
│ Key │              Action               │
├─────┼───────────────────────────────────┤
│ j   │ Move down                         │
├─────┼───────────────────────────────────┤
│ k   │ Move up                           │
├─────┼───────────────────────────────────┤
│ l   │ Open folder, or open file by type │
├─────┼───────────────────────────────────┤
│ h   │ Go to parent folder               │
├─────┼───────────────────────────────────┤
│ q   │ Quit                              │
├─────┼───────────────────────────────────┤
│ r   │ Rename (not implemented)          │
├─────┼───────────────────────────────────┤
│ p   │ Preview (not implemented)         │
├─────┼───────────────────────────────────┤
│ o   │ Open (not implemented)            │
├─────┼───────────────────────────────────┤
│ s   │ Search (not implemented)          │
└─────┴───────────────────────────────────┘

## How files open

Selecting a file checks its extension:

- Images (.png, .jpg, .gif, ...) open in imv as a detached child, s
  the TUI keeps running
- A blocklist of known binary and document formats is refused outri
- Everything else opens in nvim, blocking until you quit it
- As of now the base path is hard coded but will be configurable in the future. This is going to be $HOME

SIGCHLD is ignored in main so detached image viewers do not become zombies.

## Design notes

The terminal is put in raw mode with ECHO, ICANON, IEXTEN, ISIG, IXON
and OPOST cleared, and the alternate screen buffer (\x1b[?1049h) is
so the shell scrollback is preserved on exit. disableRawMode is registered
with atexit, so termios is restored on any normal exit path.

Scroll state is three fields on the global Config E: cx (cursor row
screen, 1 based), window_offset (index of the first visible entry), and
cur_row (index into entries).

## Known issues

- No window resize handling, no SIGWINCH handler
- Rename, preview, search, and open are stubs
- Missing row at the bottom causes flicker
- Cannot hide dotfiles
