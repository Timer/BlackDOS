#include "blackdos.h"

int isUpper(char *);
void to_filename(char *, char *);
char *str_term_next(char *);
int str_length(char *);
int str_begins(char *, char *);
char *trimFront(char *);
void do_copy(char *, char *);

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
    } else if (str_begins(command, "del")) {
      // TODO: del
    } else if (str_begins(command, "dir")) {
      // TODO: dir
    } else if (str_begins(command, "echo")) {
      /* skip echo + space */
      PRINTS(trimFront(command + 4));
      PRINTS("\r\n\0");
    } else if (str_begins(command, "help")) {
      // TODO: help
    } else if (str_begins(command, "run")) {
      interrupt(33, 4, trimFront(command + 3), 4, 0);
    } else if (str_begins(command, "tweet")) {
      // TODO: tweet
    } else if (str_begins(command, "type")) {
      // TODO: type
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
