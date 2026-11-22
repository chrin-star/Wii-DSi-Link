#include <gccore.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TCP_PORT 4242

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    VIDEO_Init();
    WPAD_Init();

    GXRModeObj *mode = VIDEO_GetPreferredMode(NULL);
    void *framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(mode));
    console_init(framebuffer, 20, 20, mode->fbWidth, mode->xfbHeight,
                 mode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(mode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (mode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    printf("\x1b[2;0H");
    printf("Wii-DSi-Link - TCP server test\n");
    printf("--------------------------------\n");
    printf("Connecting Wii to saved Wi-Fi...\n\n");

    s32 result = net_init();
    if (result != 0) {
        printf("Wi-Fi initialization failed: %d\n", (int)result);
        printf("\nPress HOME to exit.\n");
        while (1) {
            WPAD_ScanPads();
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
            VIDEO_WaitVSync();
        }
        return 0;
    }

    struct in_addr ip;
    ip.s_addr = net_gethostip();
    printf("Wi-Fi connection initialized.\n");
    if (ip.s_addr != 0)
        printf("Wii IP: %s\n", inet_ntoa(ip));
    else
        printf("Could not obtain Wii IP address.\n");

    printf("\nOpening TCP port %d...\n", TCP_PORT);
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server < 0) {
        printf("socket() failed: %d\n", errno);
        net_deinit();
        return 0;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(TCP_PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("bind() failed: %d\n", errno);
        close(server);
        net_deinit();
        return 0;
    }

    if (listen(server, 1) < 0) {
        printf("listen() failed: %d\n", errno);
        close(server);
        net_deinit();
        return 0;
    }

    printf("TCP SERVER: READY\n");
    printf("Waiting for DSi at 192.168.1.3...\n");
    printf("Keep this screen open.\n");

    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int connection = accept(server, (struct sockaddr *)&client, &client_len);

    if (connection < 0) {
        printf("\naccept() failed: %d\n", errno);
    } else {
        char message[128];
        memset(message, 0, sizeof(message));
        int received = recv(connection, message, sizeof(message) - 1, 0);
        if (received > 0) {
            message[received] = '\0';
            printf("\nDSi connected: %s\n", inet_ntoa(client.sin_addr));
            printf("Message from DSi:\n%s\n", message);
            const char reply[] = "HELLO FROM WII";
            int sent = send(connection, reply, sizeof(reply) - 1, 0);
            if (sent > 0)
                printf("\nReply sent to DSi.\n");
            else
                printf("\nCould not send reply. Error: %d\n", errno);
            printf("\nTCP TEST: MESSAGE RECEIVED\n");
        } else {
            printf("\nDSi connected, but no message received.\n");
            printf("recv() result: %d error: %d\n", received, errno);
        }
        close(connection);
    }

    close(server);
    printf("\nPress HOME to exit.\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
        VIDEO_WaitVSync();
    }

    net_deinit();
    return 0;
}
