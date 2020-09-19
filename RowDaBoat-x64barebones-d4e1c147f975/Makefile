
all:  bootloader kernel userland image
buddy:	bootloader kernelbuddy userland imagebuddy

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all

kernelbuddy:
	cd Kernel; make buddy

userland:
	cd Userland; make all

image: kernel bootloader userland
	cd Image; make all

imagebuddy: kernelbuddy bootloader userland
	cd Image; make all

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Userland; make clean

.PHONY: bootloader image imagebuddy collections kernel kernelbuddy userland buddy all clean 
