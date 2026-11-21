#include <nds.h>
#include <stdio.h>

int main(void)
{
    consoleDemoInit();
    iprintf("\nWii-DSi-Link - DSi test\n");
    iprintf("------------------------\n");
    iprintf("DSi homebrew started successfully.\n");
    iprintf("Network messaging is not implemented yet.\n\n");
    iprintf("Press START to exit.\n");

    while (1) {
        scanKeys();
        if (keysDown() & KEY_START)
            break;
        swiWaitForVBlank();
    }
    return 0;
}
