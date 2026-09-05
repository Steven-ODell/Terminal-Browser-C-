#include "inputs.h"
#include "path_handle.h"
#include "term_set.h"
#include <filesystem>
#include <iostream>

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
  case 'q': {
    if (E.state == Config::State::Browser) {
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  case 'Q': {
    if (E.state == Config::State::Browser) {
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
    } else if (E.state == Config::State::Rename) {
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
    }
  }
  // Rename
  case 'r': {
    if (E.state == Config::State::Browser) {
      E.state = Config::State::Rename;
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Preview
  case 'p': {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Open
  case 'o': {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
      break;
    } else if (E.state == Config::State::Browser) {
      openCurrentPath(E.entries[E.cur_row - 1]);
      break;
    }
  }
  // Search
  case 's': {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }

  // Backspace Key
  case '\x7f': {
    if (E.state == Config::State::Browser) {
      loadPreviousPath(E.full_path);
    } else if (E.state == Config::State::Rename) {
      E.new_name.pop_back();
    }
    break;
  }

  // Enter Key
  case '\r': {
    if (E.state == Config::State::Rename) {
      renamePath();
    } else if (E.state == Config::State::Browser) {
      openCurrentPath(E.entries[E.cur_row - 1]);
    }
    break;
  }

  // Inputs for navigation
  // Open folder or file
  case 'l': {
    if (E.state == Config::State::Browser) {
      openCurrentPath(E.entries[E.cur_row - 1]);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Up
  case 'k': {
    if (E.state == Config::State::Browser) {
      if (E.cx > 1) {
        E.cx--;
        E.cur_row--;
      } else if (E.window_offset > 0) {
        E.window_offset--;
        E.cur_row--;
      }
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Down
  case 'j': {
    if (E.state == Config::State::Browser) {
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
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Back to previous folder (block if in /home/sao)
  case 'h': {
    if (E.state == Config::State::Browser) {
      loadPreviousPath(E.full_path);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  default: {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
  }
  }
}
