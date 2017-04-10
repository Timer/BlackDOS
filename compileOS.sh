#!/bin/sh

# Reset floppy
dd if=/dev/zero of=floppya.img bs=512 count=2880

# Load bootloader
dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc

# Load map and dir
dd if=map.img of=floppya.img bs=512 count=1 seek=1 conv=notrunc
dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc

# Compile kernel
bcc -ansi -c -o kernel.o kernel.c

# Link
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o

# Add to floppy
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3

# Compile load file
gcc -o loadFile loadFile.c

# Compile cal
bcc -ansi -c -o cal.o cal.c
as86 lib.asm -o lib_asm.o
ld86 -o cal -d cal.o lib_asm.o

# Compile shell
bcc -ansi -c -o Shell.o Shell.c
ld86 -o Shell -d Shell.o lib_asm.o

# Load Files
./loadFile msg
./loadFile test1
./loadFile test2
./loadFile cal
./loadFile Shell
