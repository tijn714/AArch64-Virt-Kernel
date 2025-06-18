CROSS=aarch64-elf-
CC=$(CROSS)gcc
AS=$(CROSS)as
LD=$(CROSS)ld
OBJC=$(CROSS)objcopy

# Custom flags for customization
# Already built-in flags:
# -DDEBUG_ENABLED : Enables debug features in the kernel (Log everything to the UART console) - NOT recommended for production builds

CUSTOM_CFLAGS=-DDEBUG_ENABLED 

BUILD_FOR=QEMU_VIRT

C_FILES = $(wildcard drivers/*.c lib/*.c src/*.c fs/*.c)
S_FILES = $(wildcard drivers/*.S)
O_FILES = $(C_FILES:.c=.o) $(S_FILES:.S=.o)

QEMU_SYS_CPU=cortex-a72
QEMU_SYS_RAM=4096M
QEMU_SYS_DTB=virt.dtb


QEMU_SYS_FLAGS=-cpu $(QEMU_SYS_CPU) \
			   -m $(QEMU_SYS_RAM) \
			   -vga none \
			   -device virtio-gpu \
			   -device ramfb \
			   -display cocoa \
			   -serial stdio

all: clean kernel8.img

start.o: src/start.S
	@echo "AS start.o"
	@$(AS) src/start.S -o start.o

%.o: %.c
	@echo "CC $@ -> $< | $(CUSTOM_CFLAGS)"
	@$(CC) -c -ffreestanding -Wall -Werror -O2 -fno-stack-protector -Iinclude $(CUSTOM_CFLAGS) $< -o $@

%.o: %.S
	@echo "AS $@ -> $<"
	@$(AS) $< -o $@

kernel8.img: $(O_FILES) start.o
	@echo "LD $(O_FILES) start.o -> kernel8.elf"
	@$(LD) -nostdlib -T src/linker.ld $(O_FILES) start.o -o kernel8.elf
	@echo "OBJC kernel8.elf -> kernel8.img"
	@$(OBJC) -O binary kernel8.elf kernel8.img

	@if [ -f kernel8.img ]; then \
		echo "kernel8.img created successfully"; \
	else \
		echo "Error: kernel8.img not created"; \
		exit 1; \
	fi

clean:
	rm -rf *.img *.elf *.o
	rm -rf $(O_FILES)
	rm -rf *.dtb *.dts

dtb: kernel8.img
	@echo "generating dtb file '#{QEMU_SYS_DTB}'"
	@qemu-system-aarch64 -machine virt,dumpdtb=$(QEMU_SYS_DTB) \
							$(QEMU_SYS_FLAGS) \
      						-kernel kernel8.img


dts: dtb
	dtc -I dtb -O dts -o virt.dts virt.dtb

run-dtb: dtb
	qemu-system-aarch64 -machine virt \
						$(QEMU_SYS_FLAGS) \
						-kernel kernel8.img \
						-dtb $(QEMU_SYS_DTB)

run: kernel8.img
	qemu-system-aarch64 -machine virt \
						$(QEMU_SYS_FLAGS) \
						-kernel kernel8.img