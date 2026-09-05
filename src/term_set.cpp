#include "term_set.h"
#include <filesystem>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

Config E;

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 4);

  perror(s);
  exit(1);
}

void disableRawMode() {
  write(STDOUT_FILENO, "\x1b[?1049l", 8);
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &E.orig_termios);
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  write(STDOUT_FILENO, "\x1b[?1049h", 8);
}

void drawRows() {
  for (int i = 0; i < E.screen_rows; i++) {
    int index = i + E.window_offset;
    if (index >= E.entries.size())
      break;
    std::string buf;
    buf = "» " + E.entries[index].path().filename().string();
    write(STDOUT_FILENO, buf.c_str(), buf.size());
    if (i < E.screen_rows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}

int getWinSize(int *rows, int *cols) {
  struct winsize ws;

  // If the window doesnt exist or is invalid then exit
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;

    // Pull the terminal window dimensions
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void refreshScreen() {
  // Clear screen and set cursor to top corner
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 4);

  drawRows();

  // Put the cursor on the correct row with E.cx
  std::string seq = "\x1b[" + std::to_string(E.cx) + ";1H";
  write(STDOUT_FILENO, seq.c_str(), seq.size());
}

void initExplorer() {
  E.cx = 1;

  // If the window comes back as -1 or invalid then "die"
  if (getWinSize(&E.screen_rows, &E.screen_cols) == -1)
    die("getWinSize");

  // If it isnt an invalid screen size then load the path into the entries
  loadEntriesFrPath(E.full_path);
}
