include Makefile.inc

KERNEL=kernel.bin

SOURCES=$(wildcard *.c)
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_INTERRUPTS=$(wildcard interruptions/*.c)
SOURCES_INTERRUPTS_ASM=$(wildcard interruptions/*.asm)
SOURCES_DRIVERS=$(wildcard drivers/*.c)
SOURCES_DRIVERS_ASM=$(wildcard drivers/*.asm)
SOURCES_SYSCALLS=$(wildcard syscalls/*.c)

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
OBJECTS_INTERRUPTS=$(SOURCES_INTERRUPTS:.c=.o)
OBJECTS_INTERRUPTS_ASM=$(SOURCES_INTERRUPTS_ASM:.asm=.o)
OBJECTS_DRIVERS=$(SOURCES_DRIVERS:.c=.o)
OBJECTS_DRIVERS_ASM=$(SOURCES_DRIVERS_ASM:.asm=.o)
OBJECTS_SYSCALLS=$(SOURCES_SYSCALLS:.c=.o)

LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(OBJECTS_INTERRUPTS) $(OBJECTS_INTERRUPTS_ASM) $(OBJECTS_DRIVERS) $(OBJECTS_DRIVERS_ASM) $(OBJECTS_SYSCALLS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $^

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I./interruptions -I./drivers -I./syscalls -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o *.bin interruptions/*.o drivers/*.o syscalls/*.o

.PHONY: all clean
	