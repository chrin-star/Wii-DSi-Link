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

/* Register the BSD socket device used by socket(), bind(), listen(), etc. */
int socInit(void);
int socExit(void);

#define TCP_PORT 4242
#define PACKET_SIZE 5

static int receive_packet(int sock, unsigned char *packet)
{
    int total = 0;
    while (total < PACKET_SIZE) {
        int n = recv(sock, packet + total, PACKET_SIZE - total, 0);
        if (n <= 0)
            return n;
        total += n;
    }
    return total;
}

static void draw_cursor(int x, int y, int *old_col, int *old_row, int *visible)
{
    int col = 2 + (x * 77) / 255;
    int row = 6 + (y * 20) / 191;

    if (*visible) {
        printf("\x1b[%d;%dH ", *old_row, *old_col);
    }

    printf("\x1b[%d;%dH+", row, col);
    fflush(stdout);

    *old_col = col;
    *old_row = row;
    *visible = 1;
}

static void hide_cursor(int *old_col, int *old_row, int *visible)
{
    if (*visible) {
        printf("\x1b[%d;%dH ", *old_row, *old_col);
        fflush(stdout);
        *visible = 0;
    }
}

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

    printf("\x1b[2J\x1b[1;1H");
    printf("Wii-DSi-Link - TOUCHPAD TEST\n");
    printf("----------------------------------------\n");
    printf("Move the stylus on the DSi touch screen.\n");
    printf("The + cursor follows your finger/stylus.\n\n");
    printf("Connecting Wii to saved Wi-Fi...\n");

    s32 result = net_init();
    if (result != 0) {
        printf("Wi-Fi initialization failed: %d\n", (int)result);
        goto wait_exit_no_network;
    }

    if (socInit() != 0) {
        printf("Socket layer initialization failed.\n");
        net_deinit();
        goto wait_exit_no_network;
    }

    struct in_addr ip;
    ip.s_addr = net_gethostip();
    printf("Wi-Fi ready. Wii IP: %s\n", ip.s_addr ? inet_ntoa(ip) : "unavailable");
    printf("Opening TCP port %d...\n", TCP_PORT);

    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server < 0) {
        printf("socket() failed: %d\n", errno);
        socExit();
        net_deinit();
        goto wait_exit_no_network;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(TCP_PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("bind() failed: %d\n", errno);
        close(server);
        socExit();
        net_deinit();
        goto wait_exit_no_network;
    }

    if (listen(server, 1) < 0) {
        printf("listen() failed: %d\n", errno);
        close(server);
        socExit();
        net_deinit();
        goto wait_exit_no_network;
    }

    printf("TCP SERVER: READY\n");
    printf("Waiting for DSi at 192.168.1.3...\n");

    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int connection = accept(server, (struct sockaddr *)&client, &client_len);

    if (connection < 0) {
        printf("accept() failed: %d\n", errno);
    } else {
        unsigned char packet[PACKET_SIZE];
        int old_col = 0, old_row = 0, visible = 0;
        printf("\x1b[2J\x1b[1;1H");
        printf("Wii-DSi-Link - TOUCHPAD ACTIVE\n");
        printf("----------------------------------------\n");
        printf("DSi connected: %s\n", inet_ntoa(client.sin_addr));
        printf("Move the stylus on the DSi.\n");
        printf("Press START on DSi to disconnect.\n");

        while (1) {
            int n = receive_packet(connection, packet);
            if (n <= 0)
                break;

            if (packet[0] == 1) {
                int x = ((int)packet[1] << 8) | packet[2];
                int y = ((int)packet[3] << 8) | packet[4];
                if (x > 255) x = 255;
                if (y > 191) y = 191;
                draw_cursor(x, y, &old_col, &old_row, &visible);
            } else if (packet[0] == 0) {
                hide_cursor(&old_col, &old_row, &visible);
            }
        }

        hide_cursor(&old_col, &old_row, &visible);
        printf("\x1b[30;1HDSi disconnected.\n");
        close(connection);
    }

    close(server);
    printf("\nPress HOME on the Wii Remote to exit.\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;
        VIDEO_WaitVSync();
    }

    socExit();
    net_deinit();
    return 0;

wait_exit_no_network:
    printf("\nPress HOME to exit.\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;
        VIDEO_WaitVSync();
    }
    return 0;
}
