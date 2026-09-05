#include "inputs.h"
#include "path_handle.h"

namespace fs = std::filesystem;

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
  // Quit
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
  // Open
  case 'o': {
    break;
  }
  // Search
  case 's': {
    break;
  }
  // Inputs for navigation
  // Open folder or file
  case 'l': {
    fs::directory_entry folder_being_opened = E.entries[E.cur_row - 1];
    loadEntriesFrPath(folder_being_opened);
    write(STDOUT_FILENO, "\x1b[H", 4);
    break;
  }
  // Up
  case 'k': {
    if (E.cx > 1) {
      E.cx--;
      E.cur_row--;
    } else if (E.window_offset > 0) {
      E.window_offset--;
      E.cur_row--;
    }
    break;
  }
  // Down
  case 'j': {
    if (E.cur_row < (E.entries.size())) {
      if (E.cx < E.screen_rows - (E.screen_rows / 2)) {
        E.cx++;
        E.cur_row++;
      } else if (E.window_offset + E.screen_rows < E.entries.size()) {
        E.window_offset++;
        E.cur_row++;
      } else if (E.cx < E.screen_rows) {
        E.cx++;
        E.cur_row++;
      }
    }
    break;
  }
  // Back to previous folder (block if in /home/sao)
  case 'h': {
    loadPreviousPath(E.full_path);
    break;
  }
  }
}
