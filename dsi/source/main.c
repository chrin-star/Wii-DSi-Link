#include <nds.h>
#include <dswifi9.h>
#include <netinet/in.h>
#include <stdio.h>

int main(void)
{
    struct in_addr ip, gateway, mask, dns1, dns2;

    consoleDemoInit();
    iprintf("\nWii-DSi-Link - Wi-Fi test\n");
    iprintf("--------------------------\n");
    iprintf("Connecting DSi XL to saved Wi-Fi...\n");
    iprintf("Please wait.\n\n");

    if (!Wifi_InitDefault(WFC_CONNECT)) {
        iprintf("Wi-Fi connection failed.\n");
        iprintf("Check the DSi Internet settings.\n");
    } else {
        ip = Wifi_GetIPInfo(&gateway, &mask, &dns1, &dns2);
        iprintf("Wi-Fi connected successfully!\n");
        iprintf("DSi IP: %s\n", inet_ntoa(ip));
        iprintf("Gateway: %s\n", inet_ntoa(gateway));
    }

    iprintf("\nPress START to exit.\n");
    while (1) {
        scanKeys();
        if (keysDown() & KEY_START)
            break;
        swiWaitForVBlank();
    }

    return 0;
}
