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
    printf("\nWii-DSi-Link - DSi Wi-Fi test\n");
    printf("--------------------------------\n");
    printf("Initializing DSi Wi-Fi mode...\n");

    if (!Wifi_InitDefault(INIT_ONLY | WIFI_ATTEMPT_DSI_MODE)) {
        printf("DSi Wi-Fi initialization failed.\n");
        printf("\nPress START to exit.\n");
        while (1) {
            swiWaitForVBlank();
            scanKeys();
            if (keysDown() & KEY_START)
                return 0;
        }
    }

    printf("DSi Wi-Fi mode initialized.\n");
    printf("Connecting to saved network...\n");
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
            printf("Wii-DSi-Link - DSi Wi-Fi test\n");
            printf("--------------------------------\n");
            printf("Connecting to saved network...\n");
            printf("Connection status: %d\n", status);
            printf("Elapsed: %d seconds\n", frames / 60);
            printf("\nSTART: cancel\n");
        }
        frames++;
    }

    consoleClear();
    printf("Wii-DSi-Link - DSi Wi-Fi test\n");
    printf("--------------------------------\n");

    if (status == ASSOCSTATUS_ASSOCIATED) {
        ip = Wifi_GetIPInfo(&gateway, &mask, &dns1, &dns2);
        printf("Wi-Fi connected successfully!\n");
        printf("DSi IP: %s\n", inet_ntoa(ip));
        printf("Gateway: %s\n", inet_ntoa(gateway));
        printf("\nWii IP: 192.168.1.5\n");
    } else if (status == ASSOCSTATUS_CANNOTCONNECT) {
        printf("Could not connect to saved Wi-Fi.\n");
        printf("Association status: %d\n", status);
        printf("Check saved network settings.\n");
    } else {
        printf("Connection timed out or cancelled.\n");
        printf("Last status: %d\n", status);
    }

    printf("\nPress START to exit.\n");
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;
    }
    return 0;
}
