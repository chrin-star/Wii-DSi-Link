#include <nds.h>
#include <dswifi9.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#define TARGET_SSID "FIBERNET"
#define SCAN_FRAMES 1800
#define CONNECT_TIMEOUT_FRAMES 3600
#define DSI_TEST_IP "192.168.1.3"
#define ROUTER_IP   "192.168.1.1"
#define SUBNET_MASK "255.255.255.0"
#define DNS_SERVER "8.8.8.8"
#define WII_IP "192.168.1.5"
#define TCP_PORT 4242
#define PACKET_SIZE 5

static void wait_for_start(void)
{
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            break;
    }
}

static void show_result(const char *title, const char *detail)
{
    consoleClear();
    printf("Wii-DSi-Link - TOUCHPAD\n");
    printf("--------------------------------\n");
    printf("%s\n\n", title);
    printf("%s\n", detail);
    printf("\nPress START to exit.\n");
    wait_for_start();
}

static int send_all(int sock, const unsigned char *data, int length)
{
    int total = 0;
    while (total < length) {
        int n = send(sock, data + total, length - total, 0);
        if (n <= 0)
            return -1;
        total += n;
    }
    return total;
}

int main(void)
{
    int frames = 0;
    int count;
    int i;
    int found = 0;
    int status = ASSOCSTATUS_DISCONNECTED;
    int sock;
    int was_touching = 0;
    Wifi_AccessPoint target;

    consoleDemoInit();
    printf("\nWii-DSi-Link - TOUCHPAD\n");
    printf("--------------------------------\n");
    printf("Initializing DSi Wi-Fi mode...\n");

    if (!Wifi_InitDefault(INIT_ONLY | WIFI_ATTEMPT_DSI_MODE)) {
        show_result("Wi-Fi initialization failed.", "Could not initialize DSi Wi-Fi.");
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
            printf("Wii-DSi-Link - TOUCHPAD\n");
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
        show_result("Target network not found.", "Required: FIBERNET (saved and compatible).");
        return 0;
    }

    consoleClear();
    printf("Network found: %s\n", TARGET_SSID);
    printf("Signal: %d dBm\n", target.rssi);
    printf("\nUsing static IP: %s\n", DSI_TEST_IP);
    printf("Connecting to Wi-Fi...\n");

    Wifi_SetIP(inet_addr(DSI_TEST_IP),
               inet_addr(ROUTER_IP),
               inet_addr(SUBNET_MASK),
               inet_addr(DNS_SERVER),
               0);

    if (Wifi_ConnectWfcAP(&target) != 0) {
        show_result("Could not start Wi-Fi connection.", "Wifi_ConnectWfcAP returned an error.");
        return 0;
    }

    frames = 0;
    while (frames < CONNECT_TIMEOUT_FRAMES) {
        struct in_addr gateway = {0}, mask = {0}, dns1 = {0}, dns2 = {0};
        struct in_addr ip;

        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START)
            return 0;

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
            printf("\nSTART: cancel\n");
        }
        frames++;
    }

    if (status != ASSOCSTATUS_ASSOCIATED) {
        char detail[96];
        snprintf(detail, sizeof(detail), "Final Wi-Fi status: %d\nExpected success: %d", status, ASSOCSTATUS_ASSOCIATED);
        show_result("Wi-Fi connection did not complete.", detail);
        return 0;
    }

    consoleClear();
    printf("Wi-Fi connection: SUCCESS\n");
    printf("DSi IP: %s\n", DSI_TEST_IP);
    printf("Connecting to Wii %s...\n", WII_IP);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        char detail[96];
        snprintf(detail, sizeof(detail), "socket() failed. Error: %d", errno);
        show_result("TCP SOCKET FAILED", detail);
        return 0;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(TCP_PORT);
    server.sin_addr.s_addr = inet_addr(WII_IP);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        char detail[128];
        snprintf(detail, sizeof(detail), "connect() failed: %d\nCheck that the Wii server is ready.", errno);
        close(sock);
        show_result("COULD NOT CONNECT TO WII", detail);
        return 0;
    }

    consoleClear();
    printf("Wii-DSi Link: CONNECTED!\n");
    printf("--------------------------------\n");
    printf("Touch and drag the stylus.\n");
    printf("The Wii cursor should follow.\n\n");
    printf("START: disconnect and exit\n");

    while (1) {
        touchPosition touch;
        unsigned char packet[PACKET_SIZE] = {0, 0, 0, 0, 0};
        swiWaitForVBlank();
        scanKeys();

        if (keysDown() & KEY_START)
            break;

        if (keysHeld() & KEY_TOUCH) {
            touchRead(&touch);
            packet[0] = 1;
            packet[1] = (unsigned char)((touch.px >> 8) & 0xFF);
            packet[2] = (unsigned char)(touch.px & 0xFF);
            packet[3] = (unsigned char)((touch.py >> 8) & 0xFF);
            packet[4] = (unsigned char)(touch.py & 0xFF);

            if (send_all(sock, packet, PACKET_SIZE) < 0)
                break;
            was_touching = 1;
        } else if (was_touching) {
            if (send_all(sock, packet, PACKET_SIZE) < 0)
                break;
            was_touching = 0;
        }
    }

    close(sock);
    show_result("Disconnected.", "Touchpad test finished.");
    return 0;
}
