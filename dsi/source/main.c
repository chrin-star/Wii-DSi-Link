#include <nds.h>
#include <dswifi9.h>
#include <netinet/in.h>
#include <stdio.h>

int main(void)
{
    struct in_addr ip, gateway, mask, dns1, dns2;
    int status = ASSOCSTATUS_DISCONNECTED;
    int frames = 0;

    consoleDemoInit();
    iprintf("\nWii-DSi-Link - DSi Wi-Fi test\n");
    iprintf("--------------------------------\n");
    iprintf("Initializing DSi Wi-Fi mode...\n");

    if (!Wifi_InitDefault(INIT_ONLY | WIFI_ATTEMPT_DSI_MODE)) {
        iprintf("DSi Wi-Fi initialization failed.\n");
        iprintf("\nPress START to exit.\n");
        while (1) {
            swiWaitForVBlank();
            scanKeys();
            if (keysDown() & KEY_START)
                return 0;
        }
    }

    iprintf("DSi Wi-Fi mode initialized.\n");
    iprintf("Connecting to saved network...\n");
    Wifi_AutoConnect();

    while (frames < 3600) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;

        status = Wifi_AssocStatus();
        if (status == ASSOCSTATUS_ASSOCIATED ||
            status == ASSOCSTATUS_CANNOTCONNECT)
            break;

        if ((frames % 30) == 0) {
            consoleClear();
            iprintf("Wii-DSi-Link - DSi Wi-Fi test\n");
            iprintf("--------------------------------\n");
            iprintf("Connecting to saved network...\n");
            iprintf("Connection status: %d\n", status);
            iprintf("Elapsed: %d seconds\n", frames / 60);
            iprintf("\nSTART: cancel\n");
        }
        frames++;
    }

    consoleClear();
    iprintf("Wii-DSi-Link - DSi Wi-Fi test\n");
    iprintf("--------------------------------\n");

    if (status == ASSOCSTATUS_ASSOCIATED) {
        ip = Wifi_GetIPInfo(&gateway, &mask, &dns1, &dns2);
        iprintf("Wi-Fi connected successfully!\n");
        iprintf("DSi IP: %s\n", inet_ntoa(ip));
        iprintf("Gateway: %s\n", inet_ntoa(gateway));
        iprintf("\nWii IP: 192.168.1.5\n");
    } else if (status == ASSOCSTATUS_CANNOTCONNECT) {
        iprintf("Could not connect to saved Wi-Fi.\n");
        iprintf("Association status: %d\n", status);
        iprintf("Check saved network settings.\n");
    } else {
        iprintf("Connection timed out or cancelled.\n");
        iprintf("Last status: %d\n", status);
    }

    iprintf("\nPress START to exit.\n");
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;
    }
    return 0;
}
