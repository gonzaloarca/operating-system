include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_MODULES=$(wildcard modules/*.c)
SOURCES_DRIVERS=$(wildcard drivers/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_LIB=$(wildcard lib/*.c)
SOURCES_TESTS=$(wildcard tests/*.c)

OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)

all: $(MODULE)

$(MODULE): $(SOURCES) $(OBJECTS_ASM) $(SOURCES_MODULES) $(SOURCES_DRIVERS) $(SOURCES_LIB) $(SOURCES_TESTS)
	$(GCC) $(GCCFLAGS) -I./include -T sampleCodeModule.ld _loader.c $(SOURCES) $(SOURCES_MODULES) $(SOURCES_DRIVERS) $(SOURCES_LIB) $(SOURCES_TESTS) $(OBJECTS_ASM) -o ../$(MODULE)

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf asm/*.o *.o

.PHONY: all clean print
