#include "blackdos.h"

int str_length(char *);
int str_begins(char *, char *);
char *trimFront(char *);

void main() {
  char command[255];
  int i;
  while (1) {
    PRINTS("blackdos ~(__^> \0");
    SCANS(command);
    PRINTS("\r\n\0");
    if (str_length(command) == 0) {
      /* emulate most terms and just show a new line */
      continue;
    }
    if (str_begins(command, "boot")) {
      interrupt(33, 11, 0, 0, 0);
    } else if (str_begins(command, "cls")) {
      for (i = 1; i <= 25; i++) {
        PRINTS("\r\n\0");
      }
      interrupt(33, 12, 0, 0, 0);
    } else if (str_begins(command, "copy")) {
    } else if (str_begins(command, "del")) {
    } else if (str_begins(command, "dir")) {
    } else if (str_begins(command, "echo")) {
      /* skip echo + space */
      PRINTS(trimFront(command + 4));
      PRINTS("\r\n\0");
    } else if (str_begins(command, "help")) {
    } else if (str_begins(command, "run")) {
      interrupt(33, 4, trimFront(command + 3), 4, 0);
    } else if (str_begins(command, "tweet")) {
    } else if (str_begins(command, "type")) {
    } else {
      PRINTS("Unknown command. Type `help` for help.\r\n\0");
    }
  }

  END;
}

int str_length(char *c) {
  int count = 0;
  while (*c != '\0') {
    ++count;
    ++c;
  }
  return count;
}

int str_begins(char *str1, char *str2) {
  int i;
  int length;
  length = str_length(str2);
  for (i = 0; i < length; ++i) {
    if (str1[i] != str2[i])
      return 0;
  }
  return 1;
}

char *trimFront(char *s) {
  while (*s == ' ') {
    ++s;
  }
  return s;
}
