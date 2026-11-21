#include <gccore.h>
#include <wiiuse/wpad.h>
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
    printf("Wii-DSi-Link - Wii test\n");
    printf("------------------------\n");
    printf("Wii homebrew started successfully.\n");
    printf("Wi-Fi messaging is not implemented yet.\n\n");
    printf("Press HOME on a Wii Remote to exit.\n");

    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;
        VIDEO_WaitVSync();
    }
    return 0;
}
