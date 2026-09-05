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
| `h` | Go to parent fold
| `Enter` | Same as `l` |
| `Backspace` | Same as `
| `o` | Open selected entry |
| `r` | Rename selected e
| `H` | Toggle dotfiles |
| `q` `Q` `Esc` | Quit |
| `d` | Delete (not implemented) |
| `p` | Preview (not impl
| `s` | Search (not implemented) |

In rename mode the bindings change: typing appends to the new name, `Backspace`
deletes a character, `EntEsc` cancels and returns
to browsing.

## How files open

Selecting a file checks its extension:

- Images (.png, .jpg, .gif, ...) open in `imv` as a detached child, so the TUI
  keeps running
- A blocklist of known binary and document formats is refused outright
- Everything else opens iquit it

`SIGCHLD` is ignored in `rs do not become zombies.

## Design notes

The terminal is put in ra `IEXTEN`, `ISIG`, `IXON`
and `OPOST` cleared, and the alternate screen buffer (`\x1b[?1049h`) is entered
so the shell scrollback iRawMode` is registered
with `atexit`, so termios is restored on any normal exit path.

State lives on a single global `Config E`. Scroll position is three fields:
`cx` (cursor row on scree(index of the first
visible entry), and `cur_row` (index into `entries`).

Modes are a `State` enum on the same struct. `Browser` and `BrowserHidden` are
the two browsing modes, ddEntriesFrPath` strips
dotfiles after reading the directory. Filtering happens at load time rather
than at draw time, so `enhat is on screen.

## Known issues

- No window resize handli
- Missing row at the bottom causes flicker
- Delete, preview, and se
- The parent-directory guard compares against a hardcoded `/home/sao` rather
  than `$HOME`, so it doee
- The path argument is concatenated onto the current directory instead of
  joined, so absolute patis no separator
- Only one argument is read, with no flag parsing
- `Q` falls through to thduring a rename cancels
  the rename instead of typing a `Q`
- Cancelling a rename witowser`, dropping
  `BrowserHidden` if dotfiles were toggled off
- Hidden-file state is a lag, so every new key
  binding has to check for both `Browser` and `BrowserHidden`
