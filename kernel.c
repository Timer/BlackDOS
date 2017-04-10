/* ACADEMIC INTEGRITY PLEDGE                                              */
/*                                                                        */
/* - I have not used source code obtained from another student nor        */
/*   any other unauthorized source, either modified or unmodified.        */
/*                                                                        */
/* - All source code and documentation used in my program is either       */
/*   my original work or was derived by me from the source code           */
/*   published in the textbook for this course or presented in            */
/*   class.                                                               */
/*                                                                        */
/* - I have not discussed coding details about this project with          */
/*   anyone other than my instructor. I understand that I may discuss     */
/*   the concepts of this program with other students and that another    */
/*   student may help me debug my program so long as neither of us        */
/*   writes anything during the discussion or modifies any computer       */
/*   file during the discussion.                                          */
/*                                                                        */
/* - I have violated neither the spirit nor letter of these restrictions. */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/* Signed:_____________________________________ Date:_____________        */
/*                                                                        */
/*                                                                        */
/* 3460:4/526 BlackDOS kernel, Version 1.03, Spring 2017.                 */

void printString(char *);
void readString(char *);
void clearScreen(int bx, int cx);
void writeInt(int);
void readInt(int *);
void handleInterrupt21(int, int, int, int);
void error(int);

void main() {
  char buffer[13312];
  int size;
  makeInterrupt21();

  interrupt(33, 12, 1, 3, 0);
  interrupt(33, 0, "___.   .__                 __       .___           \r\n\0",
            0, 0);
  interrupt(33, 0, "\\_ |__ |  | _____    ____ |  | __ __| _/___  ______\r\n\0",
            0, 0);
  interrupt(33, 0,
            " | __ \\|  | \\__  \\ _/ ___\\|  |/ // __ |/ _ \\/  ___/\r\n\0", 0,
            0);
  interrupt(33, 0,
            " | \\_\\ \\  |__/ /\\ \\\\  \\___|    </ /_/ ( <_> )___ \\ \r\n\0",
            0, 0);
  interrupt(33, 0,
            " |___  /____(____  /\\___  >__|_ \\____ |\\___/____  >\r\n\0", 0,
            0);
  interrupt(33, 0,
            "     \\/          \\/     \\/     \\/    \\/         \\/ \r\n\0",
            0, 0);
  interrupt(33, 0, " V. 1.03, C. 2017. Based on a project by M. Black. \r\n\0",
            0, 0);
  interrupt(33, 0,
            " Author(s): Caleb Kupetz, Emily Trenka, Joe Haddad.\r\n\r\n\0", 0,
            0);

  interrupt(33, 4, "Shell\0", 2, 0);
  interrupt(33, 0, "Bad or missing command interpreter.\r\n\0", 0, 0);
  while (1)
    ;
}

void printString(char *c) {
  /* print char until end of string is reached */
  while (*c != '\0') {
    char p = *c;
    interrupt(16, 14 * 256 + p, 0, 0, 0);
    ++c;
  }
  return;
}

void readString(char *c) {
  int count = 0;
  while (1) {
    /* read a string, printing it back as we get it */
    char *newChar = interrupt(22, 0, 0, 0, 0);
    if (newChar == 13) {
      /* enter hit */
      c[count] = '\0';
      printString("\r\n\0");
      break;
    } else if (newChar == 8) {
      /* handle backspace */
      if (count > 0) {
        --count;
        interrupt(16, 14 * 256 + 8, 0, 0, 0);
      }
    } else {
      /* new char */
      c[count++] = newChar;
      interrupt(16, 14 * 256 + newChar, 0, 0, 0);
    }
  }
  return;
}

void clearScreen(int bx, int cx) {
  int i = 0;
  while (i < 24) {
    /* tons of new lines =D */
    printString("\r\n\0");
    ++i;
  }
  /* make screen pretty */
  interrupt(16, 512, 0, 0, 0);
  if (bx < 1 || cx < 1)
    return;
  interrupt(16, 1536, 4096 * (bx - 1) + 256 * (cx - 1), 0, 6223);
  return;
}

int mod(int a, int b) {
  int x = a;
  while (x >= b)
    x = x - b;
  return x;
}

int div(int a, int b) {
  int q = 0;
  while (q * b <= a)
    q++;
  return (q - 1);
}

/* provided */
void writeInt(int x) {
  char number[6], *d;
  int q = x, r;
  if (x < 1) {
    d = number;
    *d = 0x30;
    d++;
    *d = 0x0;
    d--;
  } else {
    d = number + 5;
    *d = 0x0;
    d--;
    while (q > 0) {
      r = mod(q, 10);
      q = div(q, 10);
      *d = r + 48;
      d--;
    }
    d++;
  }
  printString(d);
}

void readInt(int *number) {
  int temp = 0;
  int i = 0;
  char line[6];
  readString(line);
  while (line[i] != '\0') {
    /* read an int, subtracting off the char value for 0 for a numerical */
    temp = temp * 10 + line[i++] - '0';
  }
  *number = temp;
}

void readSector(char *data, int absSectorNo) {
  /* compute position with provided math */
  int relSecNo = mod(absSectorNo, 18) + 1;
  int headNo = mod(div(absSectorNo, 18), 2);
  int trackNo = div(absSectorNo, 36);
  /* iterrupt for it */
  interrupt(19, 2 * 256 + 1, data, trackNo * 256 + relSecNo, headNo * 256 + 0);
}

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

void readFile(char *fileName, char *buffer, int *size) {
  char directory[512], aFile[7];
  int i, j, k, l, fileStart, fileEnd, shouldError, fileSize;

  readSector(directory, 2);

  shouldError = 1;
  /* loop through all files in dir */
  for (i = 0; i < 16; i++) {
    fileStart = i * 32;
    fileEnd = fileStart + 6;

    k = 0;
    for (j = fileStart; j < fileEnd; j++) {
      aFile[k++] = directory[j];
    }
    aFile[6] = '\0';

    /* check if it's what we want */
    if (!matchFile(aFile, fileName)) {
      /* it's not */
      continue;
    }

    /* we want this! read it */
    shouldError = 0;
    fileSize = 0;
    for (l = fileEnd; directory[l] != 0; l++) {
      readSector(buffer, directory[l]);
      buffer = buffer + 512;
      fileSize = fileSize + 1;
    }
    /* send back size */
    if (size != 0) {
      *size = fileSize;
    }
    break;
  }

  /* file not found */
  if (shouldError) {
    error(0);
  }
}

void runProgram(char *fileName, int segment) {
  char buffer[13312];
  int i;

  /* access file */
  readFile(fileName, buffer);
  segment = segment * 0x1000;
  /* load max size into mem */
  for (i = 0; i < 13312; i++) {
    putInMemory(segment, i, buffer[i]);
  }
  launchProgram(segment);
}

/* provided */
void error(int bx) {
  char errMsg0[18], errMsg1[17], errMsg2[13], errMsg3[17];

  errMsg0[0] = 70;
  errMsg0[1] = 105;
  errMsg0[2] = 108;
  errMsg0[3] = 101;
  errMsg0[4] = 32;
  errMsg0[5] = 110;
  errMsg0[6] = 111;
  errMsg0[7] = 116;
  errMsg0[8] = 32;
  errMsg0[9] = 102;
  errMsg0[10] = 111;
  errMsg0[11] = 117;
  errMsg0[12] = 110;
  errMsg0[13] = 100;
  errMsg0[14] = 46;
  errMsg0[15] = 13;
  errMsg0[16] = 10;
  errMsg0[17] = 0;
  errMsg1[0] = 66;
  errMsg1[1] = 97;
  errMsg1[2] = 100;
  errMsg1[3] = 32;
  errMsg1[4] = 102;
  errMsg1[5] = 105;
  errMsg1[6] = 108;
  errMsg1[7] = 101;
  errMsg1[8] = 32;
  errMsg1[9] = 110;
  errMsg1[10] = 97;
  errMsg1[11] = 109;
  errMsg1[12] = 101;
  errMsg1[13] = 46;
  errMsg1[14] = 13;
  errMsg1[15] = 10;
  errMsg1[16] = 0;
  errMsg2[0] = 68;
  errMsg2[1] = 105;
  errMsg2[2] = 115;
  errMsg2[3] = 107;
  errMsg2[4] = 32;
  errMsg2[5] = 102;
  errMsg2[6] = 117;
  errMsg2[7] = 108;
  errMsg2[8] = 108;
  errMsg2[9] = 46;
  errMsg2[10] = 13;
  errMsg2[11] = 10;
  errMsg2[12] = 0;
  errMsg3[0] = 71;
  errMsg3[1] = 101;
  errMsg3[2] = 110;
  errMsg3[3] = 101;
  errMsg3[4] = 114;
  errMsg3[5] = 97;
  errMsg3[6] = 108;
  errMsg3[7] = 32;
  errMsg3[8] = 101;
  errMsg3[9] = 114;
  errMsg3[10] = 114;
  errMsg3[11] = 111;
  errMsg3[12] = 114;
  errMsg3[13] = 46;
  errMsg3[14] = 13;
  errMsg3[15] = 10;
  errMsg3[16] = 0;

  switch (bx) {
  case 0:
    printString(errMsg0);
    break;
  case 1:
    printString(errMsg1);
    break;
  case 2:
    printString(errMsg2);
    break;
  default:
    printString(errMsg3);
  }
}

void stop() { launchProgram(8192); }

void writeSector(char *data, int absSectorNo) {
  /* compute position with provided math */
  int relSecNo = mod(absSectorNo, 18) + 1;
  int headNo = mod(div(absSectorNo, 18), 2);
  int trackNo = div(absSectorNo, 36);
  /* iterrupt for it */
  interrupt(19, 3 * 256 + 1, data, trackNo * 256 + relSecNo, headNo * 256 + 0);
}

void deleteFile(char *name) {
  char map[512];
  char directory[512];
  char aFile[7];
  int shouldError, i;

  readSector(map, 1);
  readSector(directory, 2);
  shouldError = 1;
  for (i = 0; i < 16; i++) {
    int fileStart = i * 32, fileEnd = fileStart + 6, k = 0;
    int j, l, index;
    for (j = fileStart; j < fileEnd; j++) {
      aFile[k] = directory[j];
      k++;
    } // make sure we're null terminated (safety)
    aFile[6] = '\0';

    /* check if it's what we want */
    if (!matchFile(aFile, name)) {
      /* it's not */
      continue;
    }

    shouldError = 0;
    directory[fileStart] = 0;
    for (l = fileEnd; directory[l] != 0; l++) {
      index = directory[l];
      map[index + 1] = 0;
    }
    break;
  }

  if (shouldError) {
    error(0);
    return;
  }
  writeSector(map, 1);
  writeSector(directory, 2);
}

int str_length(char *c) {
  int count = 0;
  while (*c != '\0') {
    ++count;
    ++c;
  }
  return count;
}

int exists(char *name) {
  char directory[512];
  char currentFile[7];
  int i;

  readSector(directory, 2);
  for (i = 0; i < 16; i++) {
    int fileStart = i * 32, fileEnd = fileStart + 6, k = 0;
    int j;
    for (j = fileStart; j < fileEnd; j++) {
      currentFile[k] = directory[j];
      k++;
    }
    currentFile[6] = '\0';

    if (matchFile(currentFile, name)) {
      return 1;
    }
  }
  return 0;
}

void writeFile(char *name, char *buffer, int numberOfSectors) {
  char map[512];
  char directory[512];
  char content[512];
  int i;
  int parts = 0;

  int nameLength = str_length(name);
  int bufferLength = str_length(buffer);

  readSector(map, 1);
  readSector(directory, 2);

  for (i = 0; i < 16; i++) {
    int fileStart = i * 32, fileEnd = fileStart + 6;
    int j, l, p, m;

    if (exists(name)) {
      error(1);
      return;
    }

    if (directory[fileStart] != 0) {
      continue;
    }
    for (j = 0; j < nameLength + 1; j++) {
      int k;
      directory[fileStart + j] = name[j];
      for (k = nameLength; k < 7; k++) {
        directory[fileStart + k] = 0;
      }
    }

    p = fileEnd;
    /* serach the diskmap */
    for (l = 0; l < 512 && parts < numberOfSectors; l++) {
      int n;
      /* check for free sector */
      if (map[l] == 255) {
        continue;
      }
      if (map[l] == 255 && parts == 511) {
        error(2);
        return;
      }

      n = 0;
      map[l] = 255;
      /* Write 512 bytes from the buffer holding the file to that sector */
      for (m = parts * 512; m < 512 + (parts * 512); m++) {
        content[n] = buffer[m];
        n++;
      }
      parts++;
      writeSector(content, l);
      m = 0;
      directory[p] = l;
      p++;
    }
    break;
  }
  writeSector(directory, 2);
  writeSector(map, 1);
}

void handleInterrupt21(int ax, int bx, int cx, int dx) {
  if (ax == 0)
    printString(bx);
  else if (ax == 1)
    readString(bx);
  else if (ax == 2)
    readSector(bx, cx);
  else if (ax == 3)
    readFile(bx, cx, dx);
  else if (ax == 4)
    runProgram(bx, cx);
  else if (ax == 5)
    stop();
  else if (ax == 6)
    writeSector(bx, cx);
  else if (ax == 7)
    deleteFile(bx);
  else if (ax == 8)
    writeFile(bx, cx, dx);
  else if (ax == 11)
    interrupt(25, 0, 0, 0, 0);
  else if (ax == 12)
    clearScreen(bx, cx);
  else if (ax == 13)
    writeInt(bx);
  else if (ax == 14)
    readInt(bx);
  else if (ax == 15)
    error(bx);
  else
    printString("Unknown interrupt\0");
}
