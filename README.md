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

Requires C++17 for `std::filesystem`, pinned in `CMakeLists.txt`. Takes an
optional path argument to open into a starting folder.

## Keys

| Key | Action |
|---|---|
| `j` | Move down |
| `k` | Move up |
| `l` | Open folder, or open file by type |
| `h` | Go to parent folder |
| `Enter` | Same as `l` |
| `Backspace` | Same as `h` |
| `o` | Open selected entry |
| `r` | Rename selected entry |
| `H` | Toggle dotfiles |
| `q` `Q` `Esc` | Quit |
| `d` | Delete (not implemented) |
| `p` | Preview (not implemented) |
| `s` | Search (not implemented) |

In rename mode the bindings change: typing appends to the new name, `
deletes a character, `Enter` commits the rename, and `Esc` cancels and returns
to browsing.

## How files open

Selecting a file checks its extension:

- Images (.png, .jpg, .gif, ...) open in `imv` as a detached child, s
  keeps running
- A blocklist of known binary and document formats is refused outrigh
- Everything else opens in `nvim`, blocking until you quit it

`SIGCHLD` is ignored in `main` so detached image viewers do not become zombies.

## Design notes

The terminal is put in raw mode with `ECHO`, `ICANON`, `IEXTEN`, `ISIG`, `IXON`
and `OPOST` cleared, and the alternate screen buffer (`\x1b[?1049h`)
so the shell scrollback is preserved on exit. `disableRawMode` is registered
with `atexit`, so termios is restored on any normal exit path.

State lives on a single global `Config E`. Scroll position is three f
`cx` (cursor row on screen, 1 based), `window_offset` (index of the first
visible entry), and `cur_row` (index into `entries`).

Modes are a `State` enum on the same struct. `Browser` and `BrowserHi
the two browsing modes, differing only in whether `loadEntriesFrPath` strips
dotfiles after reading the directory. Filtering happens at load time
than at draw time, so `entries` always holds exactly what is on screen.

## Known issues

- No window resize handling, no `SIGWINCH` handler
- Missing row at the bottom causes flicker
- Delete, preview, and search are stubs
- The parent-directory guard compares against a hardcoded `/home/sao`
  than `$HOME`, so it does nothing on any other machine
- The path argument is concatenated onto the current directory instea
  joined, so absolute paths are not handled and there is no separator
- Only one argument is read, with no flag parsing
- `Q` falls through to the `Esc` case, so pressing it during a rename cancels
  the rename instead of typing a `Q`
- Cancelling a rename with `Esc` always returns to `Browser`, dropping
  `BrowserHidden` if dotfiles were toggled off
- Hidden-file state is a browsing mode rather than a flag, so every new key
  binding has to check for both `Browser` and `BrowserHidden`
