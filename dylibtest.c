#include <stdio.h>
#include <mach/mach.h>
#include <curses.h>
#include <libproc.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <unistd.h>

static void injected() __attribute__((constructor));

uint64_t binja_address = 0x100000000;
uint64_t print_address = 0x102474B61;
uint64_t base_address = 0;

uint64_t aslr_bypass(uint64_t addr) {
    return print_address - binja_address + base_address;
}

void injected() {
    long slide = _dyld_get_image_vmaddr_slide(0);
    base_address = binja_address + slide;
    print_address = aslr_bypass(print_address);
    if (*(char*)print_address != 0x55) return;
    while (true) {
        clear();
        printf("Successfully Attached!\n");
        sleep(500);
    }
}