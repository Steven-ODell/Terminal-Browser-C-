#include "term_set.h"
#include <filesystem>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

void loadEntriesFrPath(fs::path new_path) {
  if (fs::is_directory(new_path)) {
    E.entries.clear();
    E.cx = 1;
    E.cur_row = 1;
    E.window_offset = 0;
    for (const auto &entry : fs::directory_iterator(new_path)) {
      E.entries.push_back(entry);
    }
    E.full_path.assign(new_path);
  } else {
    checkIfFile(new_path);
  }
}

void loadPreviousPath(fs::path cur_path) {
  if (fs::exists(cur_path.parent_path())) {
    if (cur_path != "/home/sao") {
      E.full_path.assign(cur_path.parent_path());
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
