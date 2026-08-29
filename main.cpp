#include "term_set.h"
#include <asm-generic/ioctls.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*
 Create a file explorer that works with ansi codes for terminal and cursor
 controls
 -Functional file browser
 -Nvim controls
 -open into a starting folder as an argument

 -Display files in 1 column
 [x]Make column have a cursor on the left side

 -When you select a file, check type and then open in nvim if applicable
 -Make a way for search for files/folders
 -Rename folders
 m
 the terminal for the preview line pull length
 */

int main() {
  // Set the terminal to "Raw" mode
  enableRawMode();
  // Init the explorer sceen
  initExplorer();

  // Wait for user input
  while (1) {
    refreshScreen();
    processKeypress();
  }

  return 0;
}
