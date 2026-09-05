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
  case 'q':
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  // Rename
  case 'r': {
    if (E.state == Config::State::Browser) {
      E.state = Config::State::Rename;
      write(STDOUT_FILENO, "\x1b[2J", 4);
      std::cout << "Rename \""
                << E.entries[E.cur_row].path().filename().string()
                << "\" to:" << std::endl;
      write(STDOUT_FILENO, "\x1b[2H", 3);
      std::string new_name;
      std::cin >> new_name;
      if (new_name == "") {
        std::cout << "Rename is invalid. Cant name a folder ''" << std::endl;
        sleep(1);
        break;
      } else if (fs::exists(E.entries[E.cur_row - 1])) {
        try {
          fs::rename(E.entries[E.cur_row - 1].path().parent_path(), new_name);
        } catch (const fs::filesystem_error &e) {
          std::cerr << "Filesystem error occurred: " << e.what() << std::endl;
        }
      }
      loadEntriesFrPath(E.full_path);
      break;
    }
  }
  // Preview
  case 'p': {
    break;
  }
  // Open
  case 'o': {
    openCurrentPath(E.entries[E.cur_row - 1]);
    break;
  }
  // Search
  case 's': {
    break;
  }

  // Inputs for navigation
  // Open folder or file
  case 'l': {
    if (E.state == Config::State::Rename) {
      break;
    }
    if (E.state == Config::State::Browser) {
      openCurrentPath(E.entries[E.cur_row - 1]);
    }
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
