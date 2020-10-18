include Makefile.inc

KERNEL=kernel.bin

SOURCES=$(wildcard *.c)
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_INTERRUPTS=$(wildcard interrupts/*.c)
SOURCES_INTERRUPTS_ASM=$(wildcard interrupts/*.asm)
SOURCES_DRIVERS=$(wildcard drivers/*.c)
SOURCES_DRIVERS_ASM=$(wildcard drivers/*.asm)
SOURCES_SYSCALLS=$(wildcard syscalls/*.c)
SOURCES_PROCESS=$(wildcard process/*.c)
SOURCES_MEMMANAGER=memorymanager/memManagerV1.c
SOURCES_MEMBUDDY=memorymanager/memManagerBuddy.c

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
OBJECTS_INTERRUPTS=$(SOURCES_INTERRUPTS:.c=.o)
OBJECTS_INTERRUPTS_ASM=$(SOURCES_INTERRUPTS_ASM:.asm=.o)
OBJECTS_DRIVERS=$(SOURCES_DRIVERS:.c=.o)
OBJECTS_DRIVERS_ASM=$(SOURCES_DRIVERS_ASM:.asm=.o)
OBJECTS_SYSCALLS=$(SOURCES_SYSCALLS:.c=.o)
OBJECTS_MEMMANAGER=$(SOURCES_MEMMANAGER:.c=.o)
OBJECTS_MEMBUDDY=$(SOURCES_MEMBUDDY:.c=.o)
OBJECTS_PROCESS=$(SOURCES_PROCESS:.c=.o)

LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(OBJECTS_INTERRUPTS) $(OBJECTS_INTERRUPTS_ASM) $(OBJECTS_DRIVERS) $(OBJECTS_DRIVERS_ASM) $(OBJECTS_SYSCALLS) $(OBJECTS_MEMMANAGER) $(OBJECTS_PROCESS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $^
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $^

buddy: $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(OBJECTS_INTERRUPTS) $(OBJECTS_INTERRUPTS_ASM) $(OBJECTS_DRIVERS) $(OBJECTS_DRIVERS_ASM) $(OBJECTS_SYSCALLS) $(OBJECTS_MEMBUDDY) $(OBJECTS_PROCESS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $^
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $^


%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I./interruptions -I./drivers -I./syscalls -I./memorymanager -I./process -c $< -o $@
	$(GCC) $(GCCFLAGS) -I./include -I./interruptions -I./drivers -I./syscalls -I./memorymanager -I./process -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

	
$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o *.bin interrupts/*.o drivers/*.o syscalls/*.o memorymanager/*.o process/*.o

.PHONY: buddy all clean
	