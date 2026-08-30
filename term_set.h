#include <asm-generic/ioctls.h>
#include <fcntl.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

void die(const char *s);

struct Config {
  int cx;
  int screenrows;
  int screencols;
  int window_offset = 0;
  std::string full_path;
  struct termios orig_termios;
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

void loadEntriesFrPath(std::string path_prefix, std::string path_end);
