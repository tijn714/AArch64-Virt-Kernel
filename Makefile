CROSS=aarch64-elf-
CC=$(CROSS)gcc
AS=$(CROSS)as
LD=$(CROSS)ld
OBJC=$(CROSS)objcopy

C_FILES = $(wildcard drivers/*.c lib/*.c src/*.c)
O_FILES = $(C_FILES:.c=.o)

QEMU_SYS_CPU=cortex-a72
QEMU_SYS_RAM=4096
QEMU_SYS_DTB=virt.dtb

all: clean kernel8.img dtb run

start.o: src/start.S
	$(AS) src/start.S -o start.o

%.o: %.c
	$(CC) -c -ffreestanding -Wall -Werror -O2 -fno-stack-protector -Iinclude $< -o $@

kernel8.img: $(O_FILES) start.o
	$(LD) -nostdlib -T linker.ld $(O_FILES) start.o -o kernel8.elf
	$(OBJC) -O binary kernel8.elf kernel8.img

clean:
	rm -rf *.img *.elf *.o
	rm -rf $(O_FILES)
	rm -rf *.dtb *.dts


test-fs-disk: # since macOS lacks the mkfs command, we use docker to do so.
	docker run --rm --privileged -v "$PWD":/mnt -w /mnt ubuntu \
	  bash -c "apt update && apt install -y e2fsprogs && \
	           dd if=/dev/zero of=rootfs.ext4 bs=1M count=64 && \
	           mkfs.ext4 rootfs.ext4 && \
	           mkdir -p /mnt/tmp/ext && \
	           mount -o loop rootfs.ext4 /mnt/tmp/ext && \
	           echo 'Hello, World!' > /mnt/tmp/ext/test.txt && \
	           umount /mnt/tmp/ext"



dtb: kernel8.img
	qemu-system-aarch64 -machine virt,dumpdtb=$(QEMU_SYS_DTB) \
    						-cpu $(QEMU_SYS_CPU) \
      						-m $(QEMU_SYS_RAM) \
      						-device ramfb \
      						-device virtio-gpu-pci \
      						-display cocoa \
      						-serial stdio \
      						-kernel kernel8.img


dts: dtb
	dtc -I dtb -O dts -o virt.dts virt.dtb

run:
	qemu-system-aarch64 -machine virt \
						-cpu $(QEMU_SYS_CPU) \
  						-m $(QEMU_SYS_RAM) \
  						-device ramfb \
  						-device virtio-gpu-pci \
  						-display cocoa \
  						-serial stdio \
  						-kernel kernel8.img \
						-dtb $(QEMU_SYS_DTB)
