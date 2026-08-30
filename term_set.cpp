#include "term_set.h"
#include <filesystem>
#include <sys/ioctl.h>
#include <vector>

namespace fs = std::filesystem;

Config E;

std::vector<std::filesystem::directory_entry> entries;

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

char readKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

void processKeypress() {
  char c = readKey();
  switch (c) {
  case 'q':
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 4);
    exit(0);
    break;
    // Rename
  case 'r': {
    break;
  }
  // Preview
  case 'p': {
    break;
  }
  // Search
  case 's': {
    break;
  }
    // Inputs for navigation
    // Open folder or file
  case 'l': {
    break;
  }
  // Up
  case 'k': {
    if (E.cx > 1) {
      E.cx--;
    } else if (E.window_offset > 0) {
      E.window_offset--;
    }
    break;
  }
  // Down
  case 'j': {
    if (E.cx < E.screenrows - (E.screenrows / 2)) {
      E.cx++;
    } else if (E.window_offset + E.screenrows < entries.size()) {
      E.window_offset++;
    } else if (E.cx < E.screenrows) {
      E.cx++;
    }
    break;
  }
  // Back to previous folder (block if in /home/saoii/)
  case 'h': {
    std::string folder_being_opened = "";
    loadEntriesFrPath(E.full_path, folder_being_opened);
    break;
  }
  }
}

void drawRows() {
  for (int i = 0; i < E.screenrows; i++) {
    int index = i + E.window_offset;
    if (index >= entries.size())
      break;
    std::string buf;
    buf = "» " + entries[index].path().filename().string();
    write(STDOUT_FILENO, buf.c_str(), buf.size());
    if (i < E.screenrows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}

int getWinSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void refreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 4);

  drawRows();

  std::string seq = "\x1b[" + std::to_string(E.cx) + ";1H";
  write(STDOUT_FILENO, seq.c_str(), seq.size());
}

void initExplorer() {
  E.cx = 1;

  loadEntriesFrPath(E.full_path, "");

  if (getWinSize(&E.screenrows, &E.screencols) == -1)
    die("getWinSize");

  for (const auto &entry : fs::directory_iterator(E.full_path)) {
    entries.push_back(entry);
  }
}

void loadEntriesFrPath(std::string path_prefix, std::string path_end) {
  for (const auto &entry : fs::directory_iterator(E.full_path)) {
    entries.push_back(entry);
  }
}
