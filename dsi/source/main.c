#include <nds.h>
#include <dswifi9.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#define TARGET_SSID "FIBERNET"
#define SCAN_FRAMES 1800
#define CONNECT_TIMEOUT_FRAMES 3600

/* This address was observed on the DSi itself during its system connection test.
   The system test must not be running at the same time as this homebrew. */
#define DSI_TEST_IP "192.168.1.3"
#define ROUTER_IP   "192.168.1.1"
#define SUBNET_MASK "255.255.255.0"
#define DNS_SERVER "8.8.8.8"

static void wait_for_start(void)
{
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;
    }
}

int main(void)
{
    int frames = 0;
    int count;
    int i;
    int found = 0;
    int status = ASSOCSTATUS_DISCONNECTED;
    Wifi_AccessPoint target;

    consoleDemoInit();
    printf("\nWii-DSi-Link - Static IP test\n");
    printf("--------------------------------\n");
    printf("Initializing DSi Wi-Fi mode...\n");

    if (!Wifi_InitDefault(INIT_ONLY | WIFI_ATTEMPT_DSI_MODE)) {
        printf("DSi Wi-Fi initialization failed.\n");
        printf("\nPress START to exit.\n");
        wait_for_start();
        return 0;
    }

    printf("DSi Wi-Fi mode initialized.\n");
    printf("Searching for %s...\n", TARGET_SSID);
    Wifi_ScanMode();

    while (frames < SCAN_FRAMES) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            return 0;

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
            printf("Wii-DSi-Link - Static IP test\n");
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
        wait_for_start();
        return 0;
    }

    consoleClear();
    printf("Network found: %s\n", TARGET_SSID);
    printf("Signal: %d dBm\n", target.rssi);
    printf("\nUsing the DSi's previously observed IP:\n");
    printf("IP:      %s\n", DSI_TEST_IP);
    printf("Gateway: %s\n", ROUTER_IP);
    printf("Mask:    %s\n", SUBNET_MASK);
    printf("\nStarting connection...\n");

    /* Bypass DHCP for this diagnostic only. The address .3 was reported
       by Fing as the Nintendo DS during the DSi system connection test. */
    Wifi_SetIP(inet_addr(DSI_TEST_IP),
               inet_addr(ROUTER_IP),
               inet_addr(SUBNET_MASK),
               inet_addr(DNS_SERVER),
               0);

    if (Wifi_ConnectWfcAP(&target) != 0) {
        printf("\nCould not start connection.\n");
        printf("\nPress START to exit.\n");
        wait_for_start();
        return 0;
    }

    frames = 0;
    while (frames < CONNECT_TIMEOUT_FRAMES) {
        struct in_addr gateway = {0}, mask = {0}, dns1 = {0}, dns2 = {0};
        struct in_addr ip;

        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;

        status = Wifi_AssocStatus();
        ip = Wifi_GetIPInfo(&gateway, &mask, &dns1, &dns2);

        if (status == ASSOCSTATUS_ASSOCIATED ||
            status == ASSOCSTATUS_CANNOTCONNECT)
            break;

        if ((frames % 60) == 0) {
            consoleClear();
            printf("Connecting to %s...\n", TARGET_SSID);
            printf("--------------------------------\n");
            printf("Status: %d\n", status);
            printf("Elapsed: %d seconds\n", frames / 60);
            printf("DSi IP: %s\n", inet_ntoa(ip));
            printf("Gateway: %s\n", inet_ntoa(gateway));
            printf("\nStatic IP test (DHCP bypassed)\n");
            printf("START: cancel\n");
        }
        frames++;
    }

    consoleClear();
    printf("Wii-DSi-Link - Static IP result\n");
    printf("--------------------------------\n");
    {
        struct in_addr gateway = {0}, mask = {0}, dns1 = {0}, dns2 = {0};
        struct in_addr ip = Wifi_GetIPInfo(&gateway, &mask, &dns1, &dns2);
        printf("DSi IP: %s\n", inet_ntoa(ip));
        printf("Gateway: %s\n", inet_ntoa(gateway));
        printf("Subnet: %s\n", inet_ntoa(mask));
        printf("DNS: %s\n", inet_ntoa(dns1));
    }
    printf("Final status: %d\n", status);
    printf("Expected ASSOCIATED value: %d\n", ASSOCSTATUS_ASSOCIATED);
    printf("Expected CANNOTCONNECT value: %d\n", ASSOCSTATUS_CANNOTCONNECT);

    if (status == ASSOCSTATUS_ASSOCIATED) {
        printf("\nWIFI CONNECTION: SUCCESS\n");
        printf("DHCP was bypassed using static IP.\n");
        printf("Next: test communication with Wii.\n");
    } else if (status == ASSOCSTATUS_CANNOTCONNECT) {
        printf("\nWIFI CONNECTION: FAILED\n");
        printf("The AP connection did not complete.\n");
    } else {
        printf("\nCONNECTION TEST STOPPED/TIMED OUT\n");
        printf("The connection did not reach ASSOCIATED.\n");
    }

    printf("\nPress START to exit.\n");
    wait_for_start();
    return 0;
}
