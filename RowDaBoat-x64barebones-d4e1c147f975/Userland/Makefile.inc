GCC=gcc
GPP=g++
LD=ld
AR=ar
ASM=nasm

GCCFLAGS=-m64 -fno-exceptions -std=c99 -Wall -ffreestanding -nostdlib -fno-common -mno-red-zone -mno-mmx -fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc
ARFLAGS=rvs
ASMFLAGS=-felf64
DELETEDFLAGS = -mno-sse -mno-sse2 
