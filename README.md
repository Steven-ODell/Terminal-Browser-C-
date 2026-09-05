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
optional path argument to

## Keys

| Key | Action |
|---|---|
| `j` | Move down |
| `k` | Move up |
| `l` | Open folder, or o
| `h` | Go to parent folder |
| `Enter` | Same as `l` |
| `Backspace` | Same as `h` |
| `o` | Open selected ent
| `r` | Rename selected entry |
| `H` | Toggle dotfiles |
| `q` `Q` `Esc` | Quit |
| `d` | Delete (not imple
| `p` | Preview (not implemented) |
| `s` | Search (not imple

In rename mode the bindinthe new name, `Backspace`
deletes a character, `Enter` commits the rename, and `Esc` cancels and returns
to browsing.

## How files open

Selecting a file checks i

- Images (.png, .jpg, .gitached child, so the TUI
  keeps running
- A blocklist of known birefused outright
- Everything else opens in `nvim`, blocking until you quit it

`SIGCHLD` is ignored in `main` so detached image viewers do not become zombies.

## Design notes

The terminal is put in raw mode with `ECHO`, `ICANON`, `IEXTEN`, `ISIG`, `IXON`
and `OPOST` cleared, and `\x1b[?1049h`) is entered
so the shell scrollback is preserved on exit. `disableRawMode` is registered
with `atexit`, so termiosit path.

State lives on a single gtion is three fields:
`cx` (cursor row on screen, 1 based), `window_offset` (index of the first
visible entry), and `cur_

Modes are a `State` enum  and `BrowserHidden` are
the two browsing modes, differing only in whether `loadEntriesFrPath` strips
dotfiles after reading ths at load time rather
than at draw time, so `entries` always holds exactly what is on screen.

## Known issues

- No window resize handling, no `SIGWINCH` handler
- Missing row at the bott
- Delete, preview, and search are stubs
- The parent-directory guded `/home/sao` rather
  than `$HOME`, so it does nothing on any other machine
- The path argument is coirectory instead of
  joined, so absolute paths are not handled and there is no separator
- Only one argument is re
- `Q` falls through to the `Esc` case, so pressing it during a rename cancels
  the rename instead of t
- Cancelling a rename with `Esc` always returns to `Browser`, dropping
  `BrowserHidden` if dotf
- Hidden-file state is a browsing mode rather than a flag, so every new key
  binding has to check foHidden`
