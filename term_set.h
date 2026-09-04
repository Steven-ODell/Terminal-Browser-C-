#include <asm-generic/ioctls.h>
#include <fcntl.h>
#include <filesystem>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

void die(const char *s);

namespace fs = std::filesystem;

struct Config {
  int cx;
  int screen_rows;
  int screen_cols;
  int window_offset = 0;
  int cur_row;
  std::string full_path;
  struct termios orig_termios;
  std::vector<std::filesystem::directory_entry> entries;
};

extern Config E;

void disableRawMode();

void enableRawMode();

char readKey();

void processKeypress();

void drawRows();

int getWinSize(int *rows, int *cols);

void refreshScreen();

void initExplorer();

void loadEntriesFrPath(fs::path full_path_entries);

void checkIfFile(fs::path path_to_check);

void openInEditor(const fs::path &file);

void openInViewer(const fs::path &file);

void loadPreviousPath(std::string);
