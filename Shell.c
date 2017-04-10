#include "blackdos.h"

int isUpper(char *);
void to_filename(char *, char *);
char *str_term_next(char *);
int str_length(char *);
int str_begins(char *, char *);
char *trimFront(char *);
void do_copy(char *, char *);
void print_folder();

void main() {
  char command[255];
  char buffer[13312];
  char tweet[141];
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
      char file1[7], file2[7];
      char *trimmed;
      trimmed = trimFront(command + 4);
      to_filename(trimmed, file1);
      to_filename(trimFront(str_term_next(trimmed)), file2);
      if (isUpper(file1) || isUpper(file2)) {
        interrupt(33, 15, 1, 0, 0);
        break;
      }
      do_copy(file1, file2);
      PRINTS("Copied \0");
      PRINTS(file1);
      PRINTS(" to \0");
      PRINTS(file2);
      PRINTS(".\r\n\0");
    } else if (str_begins(command, "del")) {
      char *file = trimFront(command + 3);
      if (isUpper(file)) {
        interrupt(33, 15, 1, 0, 0);
        break;
      }
      interrupt(33, 7, file, 0, 0);
    } else if (str_begins(command, "dir")) {
      print_folder();
    } else if (str_begins(command, "echo")) {
      /* skip echo + space */
      PRINTS(trimFront(command + 4));
      PRINTS("\r\n\0");
    } else if (str_begins(command, "help")) {
      PRINTS("1. `boot` should re-boot shell\r\n\0");
      PRINTS("2. `cls` should clear the screen\r\n\0");
      PRINTS("3. `copy file1 file2` should copy a file; use `dir` to "
             "verify\r\n\0");
      PRINTS("4. `del file` should delete a file; use `dir` to verify\r\n\0");
      PRINTS("5. `dir` already tested\r\n\0");
      PRINTS("6. `echo text` should echo front-trimmed text\r\n\0");
      PRINTS("7. `help` shows this manual\r\n\0");
      PRINTS("8. `run program` runs a program, try `kitty`, `fib`, or "
             "`cal`.\r\n\0");
      PRINTS("9. `tweet file` prompts for a tweet to store in `file`. Verify "
             "with `type file`.\r\n\0");
      PRINTS("10. `type file` reads the file contents and shows them on "
             "screen, try `type msg`.\r\n\0");
      PRINTS("11. Hitting enter with no command does nothing but a new "
             "line.\r\n\0");
      PRINTS("\r\n\0");
    } else if (str_begins(command, "run")) {
      interrupt(33, 4, trimFront(command + 3), 4, 0);
    } else if (str_begins(command, "tweet")) {
      char *file = trimFront(command + 5);
      if (isUpper(file)) {
        interrupt(33, 15, 1, 0, 0);
        break;
      }
      PRINTS("Enter your tweet (140 chars max): ");
      SCANS(tweet);
      tweet[140] = '\0';
      interrupt(33, 8, file, tweet, 1);
      PRINTS("\r\n\0");
    } else if (str_begins(command, "type")) {
      interrupt(33, 3, trimFront(command + 4), buffer, 0);
      PRINTS(buffer);
      PRINTS("\r\n\0");
    } else {
      PRINTS("Unknown command. Type `help` for help.\r\n\0");
    }
  }

  END;
}

int isUpper(char *str) {
  char c;
  str = trimFront(str);
  c = *str;
  if (c >= 'A' && c <= 'Z') {
    return 1;
  }
  return 0;
}

void to_filename(char *src, char *dst) {
  int i;
  for (i = 0; i < 6; ++i) {
    if (src[i] == '\0' || src[i] == ' ') {
      break;
    }
    dst[i] = src[i];
  }
  dst[i] = '\0';
}

char *str_term_next(char *str) {
  str = trimFront(str);
  while (*str != ' ' && *str != '\0') {
    ++str;
  }
  return str;
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
  while (*s == ' ' && *s != '\0') {
    ++s;
  }
  return s;
}

/* c+p from kernel.c */
int matchFile(char *file1, char *file2) {
  int i;
  /* file names can be 6 long */
  for (i = 0; i < 6; ++i) {
    if (file1[i] != file2[i])
      return 0;
    if (file1[i] == 0)
      return 1;
  }
  return 1;
}

void do_copy(char *file1, char *file2) {
  char buffer[13312];
  char directory[512];
  int i, j, k;
  int fileStart, fileEnd, sectors = 0;
  char fn[6];

  interrupt(33, 2, directory, 2, 0);
  for (i = 0; i < 16; i++) {
    fileStart = i * 32;
    fileEnd = fileStart + 6;
    for (j = 0; j < 6; j++) {
      fn[j] = directory[fileStart + j];
    }
    if (!matchFile(fn, file1)) {
      continue;
    }
    for (k = fileEnd; directory[k] != 0; k++) {
      sectors++;
    }
    break;
  }
  interrupt(33, 3, file1, buffer);
  interrupt(33, 8, file2, buffer, sectors);
}

void print_folder() {
  char directory[512];
  char file[10];
  int i, k, j, fileStart, fileEnd;

  interrupt(33, 2, directory, 2, 0, 0);
  for (i = 0; i < 16; i++) {
    fileStart = i * 32;
    fileEnd = fileStart + 6;
    k = 0;
    file[0] = '\0';
    for (j = 1; j < 7; j++) {
      file[j] = ' ';
    }
    for (fileStart; fileStart < fileEnd; fileStart++) {
      if (directory[fileStart] != 0) {
        file[k] = directory[fileStart];
        file[k + 1] = '\0';
      }
      k++;
    }
    file[k] = '\r';
    file[k + 1] = '\n';
    file[k + 2] = '\0';
    if (!isUpper(file)) {
      PRINTS(file);
    }
  }
}
