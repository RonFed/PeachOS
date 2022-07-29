#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR        0x08
#define KERNEL_DATA_SELECTOR        0x10

#define PEACHOS_TOTAL_INTERRUPTS    512

#define KILOBYTE                    (1024)
#define MEGABYTE                    ((KILOBYTE) * (KILOBYTE))

#define PEACHOS_HEAP_SIZE_BYTES     ((100) * (MEGABYTE))
#define PEACHOS_HEAP_BLOCK_SIZE     (4096)

// Osdev Memory map
#define PEACHOS_HEAP_ADDRESS        0x01000000
#define PEACHOS_HEAP_TABLE_ADDRESS  0x00007E00
#endif