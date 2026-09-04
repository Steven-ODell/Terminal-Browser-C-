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

void loadEntriesFrPath(fs::path new_path) {
  if (fs::is_directory(new_path)) {
    E.entries.clear();
    E.cx = 1;
    E.cur_row = 1;
    E.window_offset = 0;
    for (const auto &entry : fs::directory_iterator(new_path)) {
      E.entries.push_back(entry);
    }
    E.full_path = new_path.string();
  } else {
    checkIfFile(new_path);
  }
}

void loadPreviousPath(std::string current_path_string) {
  fs::path cur_path = current_path_string;
  if (fs::exists(cur_path.parent_path())) {
    if (cur_path != "/home/sao") {
      E.full_path = cur_path.parent_path().string();
      loadEntriesFrPath(E.full_path);
      write(STDOUT_FILENO, "\x1b[1H", 4);
    } else {
      std::cout << "Cant go further back than the home directory" << std::endl;
      sleep(1);
    }
  }
}

void checkIfFile(fs::path path_to_check) {
  if (fs::is_regular_file(path_to_check)) {
    // Check if image or binary or able to be opened in nvim
    std::string EXT = path_to_check.extension();
    if (EXT == ".png" || EXT == ".jpg" || EXT == ".jpeg" || EXT == ".gif" ||
        EXT == ".webp" || EXT == ".bmp") {
      // Open with image viewer
      openInViewer(path_to_check);
    } else if (EXT == ".o" || EXT == ".a" || EXT == ".so" || EXT == ".ko" ||
               EXT == ".elf" || EXT == ".bin" || EXT == ".exe" ||
               EXT == ".dll" || EXT == ".dylib" || EXT == ".pyc" ||
               EXT == ".pyo" || EXT == ".class" || EXT == ".jar" ||
               EXT == ".wasm" || EXT == ".zip" || EXT == ".tar" ||
               EXT == ".gz" || EXT == ".bz2" || EXT == ".xz" || EXT == ".zst" ||
               EXT == ".7z" || EXT == ".rar" || EXT == ".iso" ||
               EXT == ".deb" || EXT == ".rpm" || EXT == ".mp3" ||
               EXT == ".wav" || EXT == ".flac" || EXT == ".ogg" ||
               EXT == ".opus" || EXT == ".m4a" || EXT == ".mp4" ||
               EXT == ".mkv" || EXT == ".avi" || EXT == ".mov" ||
               EXT == ".webm" || EXT == ".ttf" || EXT == ".otf" ||
               EXT == ".ttc" || EXT == ".woff" || EXT == ".woff2" ||
               EXT == ".pdf" || EXT == ".doc" || EXT == ".docx" ||
               EXT == ".xls" || EXT == ".xlsx" || EXT == ".ppt" ||
               EXT == ".pptx" || EXT == ".odt" || EXT == ".db" ||
               EXT == ".sqlite" || EXT == ".sqlite3" || EXT == ".dat" ||
               EXT == ".pack" || EXT == ".idx" || EXT == ".ch8" ||
               EXT == ".nes" || EXT == ".gb" || EXT == ".gbc" ||
               EXT == ".gba" || EXT == ".smc" || EXT == ".sfc" ||
               EXT == ".z64" || EXT == ".n64" || EXT == ".rom" ||
               EXT == ".blend" || EXT == ".stl" || EXT == ".3mf" ||
               EXT == ".fbx" || EXT == ".glb" || EXT == ".dwg") {
      std::cout << "Error this file type can not be opened with an editor"
                << std::endl;

      std::string seq = "\x1b[" + std::to_string(E.cx) + ";1H";
      write(STDOUT_FILENO, seq.c_str(), seq.size());

      sleep(1);
    } else {
      // Open Nvim to file path
      openInEditor(path_to_check);
    }
  } else {
    std::cout << "Error this file type can not be opened with an editor"
              << std::endl;

    std::string seq = "\x1b[" + std::to_string(E.cx) + ";1H";
    write(STDOUT_FILENO, seq.c_str(), seq.size());

    sleep(1);
  }
}

// Open Nvim to file path
void openInEditor(const fs::path &file) {
  disableRawMode(); // Restore termios + leave alt screen

  pid_t pid = fork();
  if (pid == -1)
    die("fork");
  if (pid == 0) {
    execlp("nvim", "nvim", file.c_str(), nullptr);
    _exit(127); // Only reached if exec failed
  }
  waitpid(pid, nullptr, 0); // Block until nvim quits

  enableRawMode();                            // Back to alt screen + raw
  getWinSize(&E.screen_rows, &E.screen_cols); // They may have resized
}

void openInViewer(const fs::path &file) {
  pid_t pid = fork();
  if (pid == -1)
    die("fork");
  if (pid == 0) {
    int null = open("/dev/null", O_WRONLY);
    dup2(null, STDOUT_FILENO);
    dup2(null, STDERR_FILENO);
    execlp("imv", "imv", file.c_str(), nullptr);
    _exit(127);
  }
  // No waitpid — imv is a Wayland window, your TUI keeps running
}
