#include <nds.h>
#include <dswifi9.h>
#include <stdio.h>

#define SCAN_FRAMES 1800
#define MAX_DISPLAY_APS 8

int main(void)
{
    int frames = 0;
    int count = 0;
    int i;

    consoleDemoInit();
    printf("\nWii-DSi-Link - DSi scan test\n");
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
    printf("Starting access point scan...\n");
    Wifi_ScanMode();

    // Allow the radio to scan channels for 30 seconds.
    while (frames < SCAN_FRAMES) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;

        if ((frames % 60) == 0) {
            count = Wifi_GetNumAP();
            consoleClear();
            printf("Wii-DSi-Link - DSi scan test\n");
            printf("--------------------------------\n");
            printf("Scanning nearby Wi-Fi...\n");
            printf("Time: %d / 30 seconds\n", frames / 60);
            printf("Networks detected: %d\n", count);
            printf("\nPlease wait...\n");
            printf("START: cancel\n");
        }
        frames++;
    }

    count = Wifi_GetNumAP();
    consoleClear();
    printf("Wii-DSi-Link - Scan results\n");
    printf("--------------------------------\n");
    printf("Networks detected: %d\n", count);
    printf("WFC = saved in DS WFC data\n");
    printf("OK = compatible with DSi mode\n");
    printf("--------------------------------\n");

    if (count == 0) {
        printf("No access points detected.\n");
        printf("Check Wi-Fi signal and retry.\n");
    } else {
        int shown = count;
        if (shown > MAX_DISPLAY_APS)
            shown = MAX_DISPLAY_APS;

        for (i = 0; i < shown; i++) {
            Wifi_AccessPoint ap;
            if (Wifi_GetAPData(i, &ap) != WIFI_RETURN_OK)
                continue;

            printf("%d. %.24s\n", i + 1, ap.ssid);
            printf("   %s | CH %d | RSSI %d\n",
                   Wifi_ApSecurityTypeString(ap.security_type),
                   ap.channel, ap.rssi);
            printf("   %s | %s\n",
                   (ap.flags & WFLAG_APDATA_CONFIG_IN_WFC) ?
                       "WFC SAVED" : "NOT WFC",
                   (ap.flags & WFLAG_APDATA_COMPATIBLE) ?
                       "COMPATIBLE" : "NOT COMPATIBLE");
        }

        if (count > MAX_DISPLAY_APS)
            printf("\nOnly first %d shown.\n", MAX_DISPLAY_APS);
    }

    printf("\nWii IP (reference): 192.168.1.5\n");
    printf("\nPress START to exit.\n");
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;
    }

    return 0;
}
