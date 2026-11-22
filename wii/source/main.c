#include <gccore.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <stdio.h>

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
    printf("Wii-DSi-Link - Wi-Fi test\n");
    printf("--------------------------\n");
    printf("Connecting Wii to saved Wi-Fi...\n\n");

    s32 result = net_init();
    if (result == 0) {
        struct in_addr ip;
        ip.s_addr = net_gethostip();

        printf("Wi-Fi connection initialized.\n");
        if (ip.s_addr != 0)
            printf("Wii IP: %s\n", inet_ntoa(ip));
        else
            printf("Could not obtain the Wii IP address.\n");
    } else {
        printf("Wi-Fi connection failed.\n");
        printf("Network error: %d\n", (int)result);
        printf("Check the Wii Internet settings.\n");
    }

    printf("\nPress HOME on a Wii Remote to exit.\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;
        VIDEO_WaitVSync();
    }

    net_deinit();
    return 0;
}
