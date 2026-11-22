#include <nds.h>
#include <dswifi9.h>
#include <stdio.h>
#include <string.h>

#define TARGET_SSID "FIBERNET"
#define SCAN_FRAMES 1800
#define CONNECT_TIMEOUT_FRAMES 3600

int main(void)
{
    int frames = 0;
    int count;
    int i;
    int found = 0;
    int status = ASSOCSTATUS_DISCONNECTED;
    Wifi_AccessPoint target;

    consoleDemoInit();
    printf("\nWii-DSi-Link - DSi connection test\n");
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
    printf("Searching for %s...\n", TARGET_SSID);
    Wifi_ScanMode();

    while (frames < SCAN_FRAMES) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;

        if ((frames % 60) == 0) {
            count = Wifi_GetNumAP();
            for (i = 0; i < count; i++) {
                Wifi_AccessPoint ap;
                if (Wifi_GetAPData(i, &ap) == WIFI_RETURN_OK &&
                    strncmp(ap.ssid, TARGET_SSID, sizeof(ap.ssid)) == 0 &&
                    (ap.flags & WFLAG_APDATA_CONFIG_IN_WFC) &&
                    (ap.flags & WFLAG_APDATA_COMPATIBLE)) {
                    target = ap;
                    found = 1;
                    break;
                }
            }

            consoleClear();
            printf("Wii-DSi-Link - DSi connection test\n");
            printf("--------------------------------\n");
            printf("Looking for: %s\n", TARGET_SSID);
            printf("Networks detected: %d\n", count);
            printf("Scanning... %d / 30 seconds\n", frames / 60);
            printf("\nSTART: cancel\n");
            if (found)
                break;
        }
        frames++;
    }

    if (!found) {
        consoleClear();
        printf("Target network not found.\n");
        printf("Required: %s\n", TARGET_SSID);
        printf("WFC SAVED + COMPATIBLE\n");
        printf("\nPress START to exit.\n");
        while (1) {
            swiWaitForVBlank();
            scanKeys();
            if (keysDown() & KEY_START)
                break;
        }
        return 0;
    }

    consoleClear();
    printf("Network found: %s\n", TARGET_SSID);
    printf("Connecting with saved WFC settings...\n");
    printf("Signal: %d dBm\n", target.rssi);

    Wifi_SetIP(0, 0, 0, 0, 0);
    if (Wifi_ConnectWfcAP(&target) != 0) {
        printf("\nCould not start connection.\n");
        printf("\nPress START to exit.\n");
        while (1) {
            swiWaitForVBlank();
            scanKeys();
            if (keysDown() & KEY_START)
                break;
        }
        return 0;
    }

    frames = 0;
    while (frames < CONNECT_TIMEOUT_FRAMES) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;

        status = Wifi_AssocStatus();
        if (status == ASSOCSTATUS_ASSOCIATED ||
            status == ASSOCSTATUS_CANNOTCONNECT)
            break;

        if ((frames % 60) == 0) {
            consoleClear();
            printf("Connecting to %s...\n", TARGET_SSID);
            printf("--------------------------------\n");
            printf("Status: %d\n", status);
            printf("Elapsed: %d seconds\n", frames / 60);
            printf("\nPlease wait...\n");
            printf("START: cancel\n");
        }
        frames++;
    }

    consoleClear();
    printf("Wii-DSi-Link - Connection result\n");
    printf("--------------------------------\n");
    if (status == ASSOCSTATUS_ASSOCIATED) {
        printf("WIFI CONNECTION: SUCCESS\n");
        printf("Network: %s\n", TARGET_SSID);
        printf("The DSi is connected to the router.\n");
        printf("Wii IP: 192.168.1.5\n");
    } else if (status == ASSOCSTATUS_CANNOTCONNECT) {
        printf("WIFI CONNECTION: FAILED\n");
        printf("Final status: %d\n", status);
    } else {
        printf("CONNECTION TEST STOPPED/TIMED OUT\n");
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
