#include "term_set.h"
#include <filesystem>
#include <iostream>
#include <sys/wait.h>

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
    fs::directory_entry folder_being_opened = E.entries[E.cx - 1];
    loadEntriesFrPath(folder_being_opened);
    write(STDOUT_FILENO, "\x1b[H", 4);
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
    } else if (E.window_offset + E.screenrows < E.entries.size()) {
      E.window_offset++;
    } else if (E.cx < E.screenrows) {
      E.cx++;
    }
    break;
  }
  // Back to previous folder (block if in /home/saoii/)
  case 'h': {
    break;
  }
  }
}

void drawRows() {
  for (int i = 0; i < E.screenrows; i++) {
    int index = i + E.window_offset;
    if (index >= E.entries.size())
      break;
    std::string buf;
    buf = "» " + E.entries[index].path().filename().string();
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

  // put the cursor on the correct row with E.cx
  std::string seq = "\x1b[" + std::to_string(E.cx) + ";1H";
  write(STDOUT_FILENO, seq.c_str(), seq.size());
}

void initExplorer() {

  E.cx = 1;

  if (getWinSize(&E.screenrows, &E.screencols) == -1)
    die("getWinSize");

  loadEntriesFrPath(E.full_path);
}

void loadEntriesFrPath(fs::path new_path) {
  if (fs::is_directory(new_path)) {
    E.entries.clear();
    E.cx = 0;
    for (const auto &entry : fs::directory_iterator(new_path)) {
      E.entries.push_back(entry);
    }
  } else {
    check_if_file(new_path);
    // TODO Open with nvim not done
  }
}

void check_if_file(fs::path path_to_check) {
  std::string file_to_check = path_to_check.filename().string();
  if (fs::is_regular_file(file_to_check)) {
    // check if image or binary or able to be opened in nvim
    std::string EXT = path_to_check.extension();
    if (EXT == ".png" || EXT == ".jpg" || EXT == ".jpeg" || EXT == ".gif" ||
        EXT == ".webp" || EXT == ".bmp") {
      // open with image viewer
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
      // open Nvim to file path
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

// open Nvim to file path
void openInEditor(const fs::path &file) {
  disableRawMode(); // restore termios + leave alt screen

  pid_t pid = fork();
  if (pid == -1)
    die("fork");
  if (pid == 0) {
    execlp("nvim", "nvim", file.c_str(), nullptr);
    _exit(127); // only reached if exec failed
  }
  waitpid(pid, nullptr, 0); // block until nvim quits

  enableRawMode();                          // back to alt screen + raw
  getWinSize(&E.screenrows, &E.screencols); // they may have resized
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
  // no waitpid — imv is a Wayland window, your TUI keeps running
}
