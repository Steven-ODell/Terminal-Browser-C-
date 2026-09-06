# Term_File_Search

A terminal file explorer built on raw ANSI escape codes and termios, with vim
style navigation. No ncurses. Written to understand how TUIs actually work at
the syscall level.

Binary target is `Cexp`. About 660 lines across `src/`.

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
| `d` | Delete selected entry |
| `H` | Toggle dotfiles |
| `q` `Q` `Esc` | Quit |
| `p` | Preview (not implemented) |
| `s` | Search (not implemented) |

Rename mode: typing appends to the new name, `Backspace` deletes a character,
`Enter` commits, `Esc` cancels and returns to browsing.

Delete mode: `y` or `Y` confirms, `n` or `N` cancels. Deletion goes through
`fs::remove`, so it removes a single file or an empty directory and reports a
`filesystem_error` for anything else.

## How files open

Selecting a file checks its extension:

- Images (.png, .jpg, .gif, ...) open in `imv` as a detached child, so the TUI
  keeps running
- A blocklist of known binary, archive, media, ROM and document formats is
  refused outright
- Everything else opens in `nvim`, blocking until you quit it

`SIGCHLD` is ignored in `main` so detached image viewers do not become zombies.
Opening an empty folder prints a message and bounces you back to the parent
rather than leaving you on a blank screen.

## Design notes

The terminal is put in raw mode with `ECHO`, `ICANON`, `IEXTEN`, `ISIG`, `IXON`
and `OPOST` cleared, and the alternate screen buffer (`\x1b[?1049h`) is entered
so the shell scrollback is preserved on exit. `disableRawMode` is registered
with `atexit`, so termios is restored on any normal exit path.

State lives on a single global `Config E`. Scroll position is three fields:
`cx` (cursor row on screen, 1 based), `window_offset` (index of the first
visible entry), and `cur_row` (index into `entries`).

Modes are a `State` enum on the same struct. `Browser` and `BrowserHidden` are
the two browsing modes, differing only in whether `loadEntriesFrPath` strips
dotfiles after reading the directory. Filtering happens at load time rather
than at draw time, so `entries` always holds exactly what is on screen.

`processKeypress` dispatches on the key first and re-checks the state inside
every case. That works, but it means `E.new_name += c` appears seventeen times
and the `Browser || BrowserHidden` pair appears ten. Inverting the dispatch,
switching on state and then on key, is the planned cleanup.

## Roadmap

1. Make dotfile hiding a flag on `Config` instead of a separate state, so it
   survives navigation and stops doubling every browser check
2. Invert the `processKeypress` dispatch to state-first
3. Search: recursive from `base_dir` down, fzf style live narrowing, with its
   own input buffer. Walk once into memory, match per keystroke against the
   cached list
4. Preview, as an exercise in buffer control
5. Batch the two to four writes per frame into a single write, ideally wrapped
   in synchronized output mode (`\x1b[?2026h` / `\x1b[?2026l`)

## Known issues

- No window resize handling, no `SIGWINCH` handler
- Missing row at the bottom causes flicker. Reduced, not fixed
- Preview and search are stubs
- `deletePath` sets the state to `BrowserHidden` unconditionally when it
  finishes, so any delete silently turns dotfile hiding on
- Cancelling a delete with `n`, or a rename with `Esc`, always returns to
  `Browser`, dropping `BrowserHidden` the other direction
- `Esc` does nothing in delete mode, so the confirm prompt can only be answered
  with `y` or `n`
- `E.entries[E.cur_row - 1]` is indexed without a bounds check in five places.
  An empty `entries` makes that out of range, and `cur_row - 1` wraps if the
  field is ever unsigned
- `\x1b` quits, so any arrow key quits, since they arrive as `\x1b[A` and so on
- `deletePath` checks `fs::exists` and then calls `fs::remove` without reading
  its return value, which is both a redundant syscall and a race
- The parent-directory guard compares against a hardcoded `/home/sao` in
  `Config::base_dir` rather than `$HOME`, so it does nothing on another machine
- The path argument is concatenated onto the current directory instead of
  joined, so absolute paths are not handled and there is no separator
- Only one argument is read, with no flag parsing
