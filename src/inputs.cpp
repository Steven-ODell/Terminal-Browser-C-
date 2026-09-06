#include "inputs.h"
#include "path_handle.h"
#include "term_set.h"
#include <filesystem>

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
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  case 'Q': {
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Esc Key
  case '\x1b': {
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
    } else if (E.state == Config::State::Rename) {
      loadEntriesFrPath(E.full_path);
      E.state = Config::State::Browser;
      E.new_name = "";
    }
    break;
  }
  // Rename
  case 'r': {
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
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
  case 'y': {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
    } else if (E.state == Config::State::Delete) {
      deletePath(E.entries[E.cur_row - 1]);
    }
    break;
  }
  case 'Y': {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
    } else if (E.state == Config::State::Delete) {
      deletePath(E.entries[E.cur_row - 1]);
    }
    break;
  }
  case 'n': {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
    } else if (E.state == Config::State::Delete) {
      loadEntriesFrPath(E.full_path);
      E.state = Config::State::Browser;
      E.del_choice = "";
    }
    break;
  }
  case 'N': {
    if (E.state == Config::State::Rename) {
      E.new_name += c;
    } else if (E.state == Config::State::Delete) {
      loadEntriesFrPath(E.full_path);
      E.state = Config::State::Browser;
      E.del_choice = "";
    }
    break;
  }
  // Delete
  case 'd': {
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      E.state = Config::State::Delete;
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Open
  case 'o': {
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      openCurrentPath(E.entries[E.cur_row - 1]);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
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
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      loadPreviousPath(E.full_path);
    } else if (E.state == Config::State::Rename) {
      if (E.new_name.size() > 0)
        E.new_name.pop_back();
    }
    break;
  }

  // Enter Key
  case '\r': {
    if (E.state == Config::State::Rename) {
      renamePath();
    } else if (E.state == Config::State::Browser ||
               E.state == Config::State::BrowserHidden) {
      openCurrentPath(E.entries[E.cur_row - 1]);
    }
    break;
  }

  // Inputs for navigation
  // Open folder or file
  case 'l': {
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      openCurrentPath(E.entries[E.cur_row - 1]);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  // Up
  case 'k': {
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
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
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
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
    if (E.state == Config::State::Browser ||
        E.state == Config::State::BrowserHidden) {
      loadPreviousPath(E.full_path);
    } else if (E.state == Config::State::Rename) {
      E.new_name += c;
    }
    break;
  }
  case 'H': {
    if (E.state == Config::State::Browser) {
      E.state = Config::State::BrowserHidden;
      loadEntriesFrPath(E.full_path);
      break;
    } else if (E.state == Config::State::BrowserHidden) {
      E.state = Config::State::Browser;
      loadEntriesFrPath(E.full_path);
      break;
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
