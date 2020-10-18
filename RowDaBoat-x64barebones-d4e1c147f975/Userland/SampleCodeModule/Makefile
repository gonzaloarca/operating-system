include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_MODULES=$(wildcard modules/*.c)
SOURCES_LIB=$(wildcard c_lib/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_TESTS=$(wildcard tests/*.c)

OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)

all: $(MODULE)

$(MODULE): $(SOURCES) $(OBJECTS_ASM) $(SOURCES_MODULES) $(SOURCES_LIB) $(SOURCES_TESTS)
	$(GCC) $(GCCFLAGS) -I./include -T sampleCodeModule.ld _loader.c $(SOURCES) $(SOURCES_MODULES) $(SOURCES_LIB) $(SOURCES_TESTS) $(OBJECTS_ASM) -o ../$(MODULE)
	$(GCC) $(GCCFLAGS) -I./include -T sampleCodeModule.ld -Wl,--oformat=elf64-x86-64 _loader.c $(SOURCES) $(SOURCES_MODULES) $(SOURCES_LIB) $(SOURCES_TESTS) $(OBJECTS_ASM) -o ../0000-sampleCodeModule.elf

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf asm/*.o *.o

.PHONY: all clean print
