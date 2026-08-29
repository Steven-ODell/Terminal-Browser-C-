#include <asm-generic/ioctls.h>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*
 Create a file explorer that works with ansi codes for terminal and cursor
 controls

 Functional file browser
 Nvim controls
 cd into a starting folder as an argument

 Display files in 1 column
 Make column have a cursor on the left side

 When you select a file, check type and then open in nvim if applicable
 Make a way for search for files/folders

 Rename folders

'p' for preview of file
 Preview will show as a view that splits the screen and outputs the height of
 the terminal for the preview line pull length
 */

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 4);

  perror(s);
  exit(1);
}

struct Config {
  int cx;
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

struct Config E;

void disableRawMode() {
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
    }
    break;
  }
  // Down
  case 'j': {
    if (E.cx < E.screenrows) {
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
    write(STDOUT_FILENO, "-\r\n", 3);
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

  if (getWinSize(&E.screenrows, &E.screencols) == -1)
    die("getWinSize");
}

int main() {
  enableRawMode();
  initExplorer();

  // Wait for user input
  while (1) {
    refreshScreen();
    processKeypress();
  }
  // s "search"
  // q "quit"
  // r "rename"
  return 0;
}
